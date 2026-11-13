#include "client.h"

#include <string.h>

#include "app_conf.h"
#include "ble.h"
#include "error.h"
#include "stm32_seq.h"
#include "tl.h"

#define CLIENT_ADDRESS_SIZE   6U
#define CLIENT_UUID_SIZE      16U
#define CLIENT_HANDLE_INVALID 0xFFFFU

#define CLIENT_SCAN_INTERVAL          32U
#define CLIENT_SCAN_WINDOW            8U
#define CLIENT_SCAN_DUPLICATES_FILTER 1U
#define CLIENT_CONN_LATENCY           0U

#define CLIENT_BUTTON_MASK     0x3FU
#define CLIENT_CCCD_UUID       0x2902U
#define CLIENT_CCCD_NOTIFY 	   0x01U
#define CLIENT_CCCD_SIZE       2U
#define CLIENT_DESC_ENTRY_SIZE 4U

static const uint8_t service_uuid[CLIENT_UUID_SIZE] = {
	0x8F, 0xE5, 0xB3, 0xD5, 0x2E, 0x7F, 0x4A, 0x98,
	0x2A, 0x48, 0x7A, 0xCC, 0x01, 0xA1, 0x00, 0x00
};

static const uint8_t characteristic_uuid[CLIENT_UUID_SIZE] = {
	0x19, 0xED, 0x82, 0xAE, 0xED, 0x21, 0x4C, 0x9D,
	0x41, 0x45, 0x22, 0x8E, 0x02, 0xA1, 0x00, 0x00
};

typedef enum {
	CONTROLLER_DISCONNECTED = 0,
	CONTROLLER_CONNECTING,
	CONTROLLER_DISCOVER_SERVICE,
	CONTROLLER_DISCOVER_CHARACTERISTIC,
	CONTROLLER_DISCOVER_CCCD,
	CONTROLLER_SUBSCRIBING,
	CONTROLLER_READING,
	CONTROLLER_CONNECTED,
	CONTROLLER_DISCONNECT_PENDING,
	CONTROLLER_DISCONNECTING
} ControllerStatus;

typedef struct {
	uint8_t address[CLIENT_ADDRESS_SIZE];
	uint16_t connection_handle;
	uint16_t service_start;
	uint16_t service_end;
	uint16_t characteristic_handle;
	uint16_t cccd_handle;
	InputButtons held;
	ControllerStatus status;
	bool initial_received;
} ControllerConnection;

static ControllerConnection controllers[INPUT_PLAYER_COUNT] = {
	[INPUT_PLAYER_1] = {
		.address = {0x96, 0x8C, 0x22, 0xE1, 0x80, 0x00},
		.connection_handle = CLIENT_HANDLE_INVALID
	},
	[INPUT_PLAYER_2] = {
		.address = {0x7E, 0x9B, 0x22, 0xE1, 0x80, 0x00},
		.connection_handle = CLIENT_HANDLE_INVALID
	}
};

static bool scanning;
static bool scan_stopping;
static bool gatt_busy;
static InputPlayer player_pending;

static uint16_t le16_read(const uint8_t *data)
{
	return (uint16_t)data[0] | ((uint16_t)data[1] << 8);
}

static void controller_reset(ControllerConnection *controller)
{
	controller->connection_handle = CLIENT_HANDLE_INVALID;
	controller->service_start = 0U;
	controller->service_end = 0U;
	controller->characteristic_handle = 0U;
	controller->cccd_handle = 0U;
	controller->held = 0U;
	controller->status = CONTROLLER_DISCONNECTED;
	controller->initial_received = false;
}

static void controller_recover(ControllerConnection *controller)
{
	ASSERT(player_pending != INPUT_PLAYER_COUNT);
	ASSERT(controller == &controllers[player_pending]);
	ASSERT(controller->connection_handle != CLIENT_HANDLE_INVALID);

	controller->held = 0U;
	controller->status = CONTROLLER_DISCONNECT_PENDING;
	gatt_busy = false;

	UTIL_SEQ_SetTask(1U << CFG_TASK_CLIENT_ID, CFG_SCH_PRIO_0);
}

static void client_task(void)
{
	tBleStatus status;

	if (scanning) {
		if (player_pending == INPUT_PLAYER_COUNT || scan_stopping) {
			return;
		}

		status = aci_gap_terminate_gap_proc(GAP_GENERAL_DISCOVERY_PROC);
		if (status != BLE_STATUS_SUCCESS) {
			PANIC("BLE scan stop failed");
		}

		scan_stopping = true;
		return;
	}

	if (player_pending == INPUT_PLAYER_COUNT) {
		bool missing = false;

		for (InputPlayer player = INPUT_PLAYER_1; player < INPUT_PLAYER_COUNT; player++) {
			if (controllers[player].status == CONTROLLER_DISCONNECTED) {
				missing = true;
				break;
			}
		}

		if (!missing) {
			return;
		}

		status = aci_gap_start_general_discovery_proc(
			CLIENT_SCAN_INTERVAL,
			CLIENT_SCAN_WINDOW,
			CFG_BLE_ADDRESS_TYPE,
			CLIENT_SCAN_DUPLICATES_FILTER
		);

		if (status != BLE_STATUS_SUCCESS) {
			PANIC("BLE scan start failed");
		}

		scanning = true;
		return;
	}

	ControllerConnection *controller = &controllers[player_pending];

	if (controller->status == CONTROLLER_DISCONNECT_PENDING) {
		status = aci_gap_terminate(controller->connection_handle, HCI_REMOTE_USER_TERMINATED_CONNECTION_ERR_CODE);

		if (status != BLE_STATUS_SUCCESS) {
			PANIC("BLE controller disconnect failed");
		}

		controller->status = CONTROLLER_DISCONNECTING;
		return;
	}

	if (controller->status == CONTROLLER_DISCONNECTING) {
		return;
	}

	if (controller->status == CONTROLLER_DISCONNECTED) {
		status = aci_gap_create_connection(
			CLIENT_SCAN_INTERVAL,
			CLIENT_SCAN_WINDOW,
			GAP_PUBLIC_ADDR,
			controller->address,
			CFG_BLE_ADDRESS_TYPE,
			CONN_P1,
			CONN_P2,
			CLIENT_CONN_LATENCY,
			SUPERV_TIMEOUT,
			CONN_L1,
			CONN_L2
		);

		if (status != BLE_STATUS_SUCCESS) {
			PANIC("BLE connection failed");
		}

		controller->status = CONTROLLER_CONNECTING;
		return;
	}

	if (controller->status == CONTROLLER_CONNECTING || gatt_busy) {
		return;
	}

	switch (controller->status) {
		case CONTROLLER_DISCOVER_SERVICE:
			status = aci_gatt_disc_primary_service_by_uuid(
				controller->connection_handle,
				UUID_TYPE_128,
				(UUID_t *)service_uuid
			);
			break;

		case CONTROLLER_DISCOVER_CHARACTERISTIC:
			status = aci_gatt_disc_char_by_uuid(
				controller->connection_handle,
				controller->service_start,
				controller->service_end,
				UUID_TYPE_128,
				(UUID_t *)characteristic_uuid
			);
			break;

		case CONTROLLER_DISCOVER_CCCD:
			status = aci_gatt_disc_all_char_desc(
				controller->connection_handle,
				controller->characteristic_handle,
				controller->service_end
			);
			break;

		case CONTROLLER_SUBSCRIBING: {
			const uint8_t value[CLIENT_CCCD_SIZE] = {
				CLIENT_CCCD_NOTIFY, 0U
			};

			status = aci_gatt_write_char_desc(
				controller->connection_handle,
				controller->cccd_handle,
				sizeof(value),
				value
			);
			break;
		}

		case CONTROLLER_READING:
			status = aci_gatt_read_char_value(
				controller->connection_handle,
				controller->characteristic_handle
			);
			break;

		default:
			PANIC("Invalid controller state");
	}

	if (status != BLE_STATUS_SUCCESS) {
		if (status == BLE_STATUS_INVALID_PARAMS || status == BLE_STATUS_INVALID_HANDLE || status == BLE_STATUS_INVALID_OPERATION) {
			PANIC("BLE GATT invalid parameters or handle");
		}

		controller_recover(controller);
		return;
	}

	gatt_busy = true;
}

static SVCCTL_EvtAckStatus_t client_gatt_event_process(void *pckt)
{
	const hci_event_pckt *event = (hci_event_pckt *)((hci_uart_pckt *)pckt)->data;

	if (event->evt != HCI_VENDOR_SPECIFIC_DEBUG_EVT_CODE) {
		return SVCCTL_EvtNotAck;
	}

	const evt_blecore_aci *aci = (evt_blecore_aci *)event->data;

	if (aci->ecode == ACI_GATT_NOTIFICATION_VSEVT_CODE) {
		const aci_gatt_notification_event_rp0 *notification = (aci_gatt_notification_event_rp0 *)aci->data;

		if (notification->Attribute_Value_Length != sizeof(InputButtons) || (notification->Attribute_Value[0] & ~CLIENT_BUTTON_MASK) != 0U) {
			return SVCCTL_EvtNotAck;
		}

		for (InputPlayer player = INPUT_PLAYER_1; player < INPUT_PLAYER_COUNT; player++) {
			ControllerConnection *controller = &controllers[player];

			if (controller->connection_handle != notification->Connection_Handle || controller->characteristic_handle != notification->Attribute_Handle) {
				continue;
			}

			if (controller->status == CONTROLLER_CONNECTED || controller->status == CONTROLLER_READING) {
				controller->held = notification->Attribute_Value[0];
			}

			return SVCCTL_EvtNotAck;
		}

		return SVCCTL_EvtNotAck;
	}

	if (player_pending == INPUT_PLAYER_COUNT) {
		return SVCCTL_EvtNotAck;
	}

	ControllerConnection *controller = &controllers[player_pending];

	switch (aci->ecode) {
		case ACI_ATT_FIND_BY_TYPE_VALUE_RESP_VSEVT_CODE: {
			const aci_att_find_by_type_value_resp_event_rp0 *response = (aci_att_find_by_type_value_resp_event_rp0 *)aci->data;

			if (controller->status != CONTROLLER_DISCOVER_SERVICE || response->Connection_Handle != controller->connection_handle || response->Num_of_Handle_Pair == 0U) {
				return SVCCTL_EvtNotAck;
			}

			const uint8_t *pair = (uint8_t *)response->Attribute_Group_Handle_Pair;

			controller->service_start = le16_read(pair);
			controller->service_end = le16_read(pair + 2U);
			return SVCCTL_EvtNotAck;
		}

		case ACI_GATT_DISC_READ_CHAR_BY_UUID_RESP_VSEVT_CODE: {
			const aci_gatt_disc_read_char_by_uuid_resp_event_rp0 *response = (aci_gatt_disc_read_char_by_uuid_resp_event_rp0 *)aci->data;

			if (controller->status != CONTROLLER_DISCOVER_CHARACTERISTIC || response->Connection_Handle != controller->connection_handle || response->Attribute_Value_Length < 3U) {
				return SVCCTL_EvtNotAck;
			}

			const uint8_t *value = response->Attribute_Value;
			const uint8_t properties = CHAR_PROP_READ | CHAR_PROP_NOTIFY;

			if ((value[0] & properties) != properties) {
				return SVCCTL_EvtNotAck;
			}

			controller->characteristic_handle = le16_read(value + 1U);
			return SVCCTL_EvtNotAck;
		}

		case ACI_ATT_FIND_INFO_RESP_VSEVT_CODE: {
			const aci_att_find_info_resp_event_rp0 *response = (aci_att_find_info_resp_event_rp0 *)aci->data;

			if (controller->status != CONTROLLER_DISCOVER_CCCD || response->Connection_Handle != controller->connection_handle || response->Format != UUID_TYPE_16) {
				return SVCCTL_EvtNotAck;
			}

			for (uint16_t offset = 0U; offset + CLIENT_DESC_ENTRY_SIZE <= response->Event_Data_Length; offset += CLIENT_DESC_ENTRY_SIZE) {
				const uint8_t *entry = &response->Handle_UUID_Pair[offset];

				if (le16_read(entry + 2U) != CLIENT_CCCD_UUID) {
					continue;
				}

				controller->cccd_handle = le16_read(entry);
				return SVCCTL_EvtNotAck;
			}

			return SVCCTL_EvtNotAck;
		}

		case ACI_ATT_READ_RESP_VSEVT_CODE: {
			const aci_att_read_resp_event_rp0 *response = (aci_att_read_resp_event_rp0 *)aci->data;

			if (controller->status != CONTROLLER_READING || response->Connection_Handle != controller->connection_handle || response->Event_Data_Length != sizeof(InputButtons) || (response->Attribute_Value[0] & ~CLIENT_BUTTON_MASK) != 0U) {
				return SVCCTL_EvtNotAck;
			}

			controller->held = response->Attribute_Value[0];
			controller->initial_received = true;
			return SVCCTL_EvtNotAck;
		}

		case ACI_GATT_PROC_COMPLETE_VSEVT_CODE: {
			const aci_gatt_proc_complete_event_rp0 *complete = (aci_gatt_proc_complete_event_rp0 *)aci->data;

			if (!gatt_busy || complete->Connection_Handle != controller->connection_handle) {
				return SVCCTL_EvtNotAck;
			}

			gatt_busy = false;

			if (complete->Error_Code != BLE_STATUS_SUCCESS) {
				controller_recover(controller);
				return SVCCTL_EvtNotAck;
			}

			switch (controller->status) {
				case CONTROLLER_DISCOVER_SERVICE:
					ASSERT(controller->service_start != 0U);
					ASSERT(controller->service_end >= controller->service_start);
					controller->status = CONTROLLER_DISCOVER_CHARACTERISTIC;
					break;

				case CONTROLLER_DISCOVER_CHARACTERISTIC:
					ASSERT(controller->characteristic_handle != 0U);
					controller->status = CONTROLLER_DISCOVER_CCCD;
					break;

				case CONTROLLER_DISCOVER_CCCD:
					ASSERT(controller->cccd_handle != 0U);
					controller->status = CONTROLLER_SUBSCRIBING;
					break;

				case CONTROLLER_SUBSCRIBING:
					controller->status = CONTROLLER_READING;
					break;

				case CONTROLLER_READING:
					ASSERT(controller->initial_received);
					controller->status = CONTROLLER_CONNECTED;
					player_pending = INPUT_PLAYER_COUNT;
					break;

				default:
					PANIC("Unexpected GATT completion");
			}

			UTIL_SEQ_SetTask(1U << CFG_TASK_CLIENT_ID, CFG_SCH_PRIO_0);
			return SVCCTL_EvtNotAck;
		}

		default:
			return SVCCTL_EvtNotAck;
	}
}

void client_init(void)
{
	player_pending = INPUT_PLAYER_COUNT;

	UTIL_SEQ_RegTask(
		1U << CFG_TASK_CLIENT_ID,
		UTIL_SEQ_RFU,
		client_task
	);

	SVCCTL_RegisterCltHandler(client_gatt_event_process);
	UTIL_SEQ_SetTask(1U << CFG_TASK_CLIENT_ID, CFG_SCH_PRIO_0);
}

void client_event_process(void *pckt)
{
	const hci_event_pckt *event = (hci_event_pckt *)((hci_uart_pckt *)pckt)->data;

	switch (event->evt) {
		case HCI_LE_META_EVT_CODE: {
			const evt_le_meta_event *meta = (evt_le_meta_event *)event->data;

			switch (meta->subevent) {
				case HCI_LE_ADVERTISING_REPORT_SUBEVT_CODE: {
					if (!scanning || player_pending != INPUT_PLAYER_COUNT) {
						return;
					}

					const hci_le_advertising_report_event_rp0 *report = (hci_le_advertising_report_event_rp0 *)meta->data;

					if (report->Num_Reports == 0U || report->Advertising_Report[0].Event_Type != ADV_IND || report->Advertising_Report[0].Address_Type != GAP_PUBLIC_ADDR) {
						return;
					}

					for (InputPlayer player = INPUT_PLAYER_1; player < INPUT_PLAYER_COUNT; player++) {
						const ControllerConnection *controller = &controllers[player];

						if (controller->status != CONTROLLER_DISCONNECTED || memcmp(report->Advertising_Report[0].Address, controller->address, CLIENT_ADDRESS_SIZE) != 0) {
							continue;
						}

						player_pending = player;
						UTIL_SEQ_SetTask(1U << CFG_TASK_CLIENT_ID, CFG_SCH_PRIO_0);
						return;
					}

					return;
				}

				case HCI_LE_CONNECTION_COMPLETE_SUBEVT_CODE: {
					const hci_le_connection_complete_event_rp0 *connection = (hci_le_connection_complete_event_rp0 *)meta->data;

					ASSERT(player_pending != INPUT_PLAYER_COUNT);

					ControllerConnection *controller = &controllers[player_pending];
					ASSERT(controller->status == CONTROLLER_CONNECTING);

					if (connection->Status != BLE_STATUS_SUCCESS) {
						controller_reset(controller);
						player_pending = INPUT_PLAYER_COUNT;
					} else {
						ASSERT(connection->Peer_Address_Type == GAP_PUBLIC_ADDR);
						ASSERT(memcmp(connection->Peer_Address, controller->address, CLIENT_ADDRESS_SIZE) == 0);

						controller->connection_handle = connection->Connection_Handle;
						controller->status = CONTROLLER_DISCOVER_SERVICE;
					}

					UTIL_SEQ_SetTask(1U << CFG_TASK_CLIENT_ID, CFG_SCH_PRIO_0);
					return;
				}

				default:
					return;
			}
		}

		case HCI_VENDOR_SPECIFIC_DEBUG_EVT_CODE: {
			const evt_blecore_aci *aci = (evt_blecore_aci *)event->data;

			if (aci->ecode != ACI_GAP_PROC_COMPLETE_VSEVT_CODE) {
				return;
			}

			const aci_gap_proc_complete_event_rp0 *complete = (aci_gap_proc_complete_event_rp0 *)aci->data;

			if (complete->Procedure_Code != GAP_GENERAL_DISCOVERY_PROC || !scanning) {
				return;
			}

			scanning = false;
			scan_stopping = false;

			UTIL_SEQ_SetTask(1U << CFG_TASK_CLIENT_ID, CFG_SCH_PRIO_0);
			return;
		}

		case HCI_DISCONNECTION_COMPLETE_EVT_CODE: {
			const hci_disconnection_complete_event_rp0 *disconnection = (hci_disconnection_complete_event_rp0 *)event->data;

			if (disconnection->Status != BLE_STATUS_SUCCESS) {
				break;
			}

			for (InputPlayer player = INPUT_PLAYER_1; player < INPUT_PLAYER_COUNT; player++) {
				ControllerConnection *controller = &controllers[player];

				if (controller->connection_handle != disconnection->Connection_Handle) {
					continue;
				}

				controller_reset(controller);

				if (player_pending == player) {
					player_pending = INPUT_PLAYER_COUNT;
					gatt_busy = false;
				}

				UTIL_SEQ_SetTask(1U << CFG_TASK_CLIENT_ID, CFG_SCH_PRIO_0);
				return;
			}

			return;
		}

		default:
			return;
	}
}

InputState client_get(const InputPlayer player)
{
	ASSERT(player >= INPUT_PLAYER_1 && player < INPUT_PLAYER_COUNT);

	const ControllerConnection *controller = &controllers[player];

	const InputState state = {
		.held = controller->status == CONTROLLER_CONNECTED ? controller->held : 0U,
		.connected = controller->status == CONTROLLER_CONNECTED
	};

	return state;
}
