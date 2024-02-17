/* 
This is a copy-edited and stripped down version to merge "usb_serial.c",
"usb_dev.*", "usb_desc.*" and "usb_mem.c". The original copyright and 
license is stated below. You most likely obtained this file by accident 
--- get the original source http://www.pjrc.com/teensy
*/

/* Teensyduino Core Library
 * http://www.pjrc.com/teensy/
 * Copyright (c) 2013 PJRC.COM, LLC.
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * 1. The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * 2. If the Software is incorporated into a build system that allows
 * selection among a list of target devices, then similar target
 * devices manufactured by PJRC.COM must be included in the list of
 * target devices and selectable in the same manner.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include "usb_serial.h"
#include "mk20dx256.h"
#include <string.h> 
#include <stdint.h>
#include <stddef.h>

// USB is only functional at 20MHz and above
#if F_CPU < 20000000
#error Unsupported cpu clock
#endif



/*
********************************************************************
 "usb_dev.c"
********************************************************************
*/


/*
********************************************************************
 "usb_desc.h/c"
********************************************************************
*/


#define ENDPOINT_UNUSED			0x00
#define ENDPOINT_TRANSIMIT_ONLY		0x15
#define ENDPOINT_RECEIVE_ONLY		0x19
#define ENDPOINT_TRANSMIT_AND_RECEIVE	0x1D


#define VENDOR_ID		0x16C0
#define PRODUCT_ID		0x0483
#define DEVICE_CLASS		2	
#define MANUFACTURER_NAME	{'T','e','e','n','s','y','d','u','i','n','o'}
#define MANUFACTURER_NAME_LEN	11
#define PRODUCT_NAME		{'U','S','B',' ','S','e','r','i','a','l'}
#define PRODUCT_NAME_LEN	10
#define EP0_SIZE		64
#define NUM_ENDPOINTS		4
#define NUM_USB_BUFFERS	12
#define NUM_INTERFACE		2
#define CDC_STATUS_INTERFACE	0
#define CDC_DATA_INTERFACE	1
#define CDC_ACM_ENDPOINT	2
#define CDC_RX_ENDPOINT         3
#define CDC_TX_ENDPOINT         4
#define CDC_ACM_SIZE            16
#define CDC_RX_SIZE             64
#define CDC_TX_SIZE             64
#define ENDPOINT2_CONFIG	ENDPOINT_TRANSIMIT_ONLY
#define ENDPOINT3_CONFIG	ENDPOINT_RECEIVE_ONLY
#define ENDPOINT4_CONFIG	ENDPOINT_TRANSIMIT_ONLY


extern const uint8_t usb_endpoint_config_table[NUM_ENDPOINTS];
typedef struct {
	uint16_t	wValue;
	uint16_t	wIndex;
	const uint8_t	*addr;
	uint16_t	length;
} usb_descriptor_list_t;
extern const usb_descriptor_list_t usb_descriptor_list[];


// **************************************************************
//   USB Device
// **************************************************************

#define LSB(n) ((n) & 255)
#define MSB(n) (((n) >> 8) & 255)

// USB Device Descriptor.  The USB host reads this first, to learn
// what type of device is connected.
static uint8_t device_descriptor[] = {
        18,                                     // bLength
        1,                                      // bDescriptorType
        0x00, 0x02,                             // bcdUSB
        DEVICE_CLASS,                           // bDeviceClass
        0,                                      // bDeviceSubClass
        0,                                      // bDeviceProtocol
        EP0_SIZE,                               // bMaxPacketSize0
        LSB(VENDOR_ID), MSB(VENDOR_ID),         // idVendor
        LSB(PRODUCT_ID), MSB(PRODUCT_ID),       // idProduct
        0x00, 0x01,                             // bcdDevice
        1,                                      // iManufacturer
        2,                                      // iProduct
        3,                                      // iSerialNumber
        1                                       // bNumConfigurations
};


// **************************************************************
//   USB Descriptor Sizes
// **************************************************************

// pre-compute the size and position of everything in the config descriptor
//
#define CONFIG_HEADER_DESCRIPTOR_SIZE	9

#define CDC_DATA_INTERFACE_DESC_POS	CONFIG_HEADER_DESCRIPTOR_SIZE
#ifdef  CDC_DATA_INTERFACE
#define CDC_DATA_INTERFACE_DESC_SIZE	9+5+5+4+5+7+9+7+7
#else
#define CDC_DATA_INTERFACE_DESC_SIZE	0
#endif

#define CONFIG_DESC_SIZE		CDC_DATA_INTERFACE_DESC_POS+CDC_DATA_INTERFACE_DESC_SIZE



// **************************************************************
//   USB Configuration
// **************************************************************

// USB Configuration Descriptor.  This huge descriptor tells all
// of the devices capbilities.
static uint8_t config_descriptor[CONFIG_DESC_SIZE] = {
        // configuration descriptor, USB spec 9.6.3, page 264-266, Table 9-10
        9,                                      // bLength;
        2,                                      // bDescriptorType;
        LSB(CONFIG_DESC_SIZE),                  // wTotalLength
        MSB(CONFIG_DESC_SIZE),
        NUM_INTERFACE,                          // bNumInterfaces
        1,                                      // bConfigurationValue
        0,                                      // iConfiguration
        0xC0,                                   // bmAttributes
        50,                                     // bMaxPower
#ifdef CDC_DATA_INTERFACE
        // interface descriptor, USB spec 9.6.5, page 267-269, Table 9-12
        9,                                      // bLength
        4,                                      // bDescriptorType
        CDC_STATUS_INTERFACE,			// bInterfaceNumber
        0,                                      // bAlternateSetting
        1,                                      // bNumEndpoints
        0x02,                                   // bInterfaceClass
        0x02,                                   // bInterfaceSubClass
        0x01,                                   // bInterfaceProtocol
        0,                                      // iInterface
        // CDC Header Functional Descriptor, CDC Spec 5.2.3.1, Table 26
        5,                                      // bFunctionLength
        0x24,                                   // bDescriptorType
        0x00,                                   // bDescriptorSubtype
        0x10, 0x01,                             // bcdCDC
        // Call Management Functional Descriptor, CDC Spec 5.2.3.2, Table 27
        5,                                      // bFunctionLength
        0x24,                                   // bDescriptorType
        0x01,                                   // bDescriptorSubtype
        0x01,                                   // bmCapabilities
        1,                                      // bDataInterface
        // Abstract Control Management Functional Descriptor, CDC Spec 5.2.3.3, Table 28
        4,                                      // bFunctionLength
        0x24,                                   // bDescriptorType
        0x02,                                   // bDescriptorSubtype
        0x06,                                   // bmCapabilities
        // Union Functional Descriptor, CDC Spec 5.2.3.8, Table 33
        5,                                      // bFunctionLength
        0x24,                                   // bDescriptorType
        0x06,                                   // bDescriptorSubtype
        CDC_STATUS_INTERFACE,                   // bMasterInterface
        CDC_DATA_INTERFACE,                     // bSlaveInterface0
        // endpoint descriptor, USB spec 9.6.6, page 269-271, Table 9-13
        7,                                      // bLength
        5,                                      // bDescriptorType
        CDC_ACM_ENDPOINT | 0x80,                // bEndpointAddress
        0x03,                                   // bmAttributes (0x03=intr)
        CDC_ACM_SIZE, 0,                        // wMaxPacketSize
        64,                                     // bInterval
        // interface descriptor, USB spec 9.6.5, page 267-269, Table 9-12
        9,                                      // bLength
        4,                                      // bDescriptorType
        CDC_DATA_INTERFACE,                     // bInterfaceNumber
        0,                                      // bAlternateSetting
        2,                                      // bNumEndpoints
        0x0A,                                   // bInterfaceClass
        0x00,                                   // bInterfaceSubClass
        0x00,                                   // bInterfaceProtocol
        0,                                      // iInterface
        // endpoint descriptor, USB spec 9.6.6, page 269-271, Table 9-13
        7,                                      // bLength
        5,                                      // bDescriptorType
        CDC_RX_ENDPOINT,                        // bEndpointAddress
        0x02,                                   // bmAttributes (0x02=bulk)
        CDC_RX_SIZE, 0,                         // wMaxPacketSize
        0,                                      // bInterval
        // endpoint descriptor, USB spec 9.6.6, page 269-271, Table 9-13
        7,                                      // bLength
        5,                                      // bDescriptorType
        CDC_TX_ENDPOINT | 0x80,                 // bEndpointAddress
        0x02,                                   // bmAttributes (0x02=bulk)
        CDC_TX_SIZE, 0,                         // wMaxPacketSize
        0,                                      // bInterval
#endif // CDC_DATA_INTERFACE

};


// **************************************************************
//   String Descriptors
// **************************************************************

// The descriptors above can provide human readable strings,
// referenced by index numbers.  These descriptors are the
// actual string data

struct usb_string_descriptor_struct {
        uint8_t bLength;
        uint8_t bDescriptorType;
        uint16_t wString[];
};

extern struct usb_string_descriptor_struct usb_string_manufacturer_name
        __attribute__ ((weak, alias("usb_string_manufacturer_name_default")));
extern struct usb_string_descriptor_struct usb_string_product_name
        __attribute__ ((weak, alias("usb_string_product_name_default")));
extern struct usb_string_descriptor_struct usb_string_serial_number
        __attribute__ ((weak, alias("usb_string_serial_number_default")));

struct usb_string_descriptor_struct string0 = {
        4,
        3,
        {0x0409}
};
struct usb_string_descriptor_struct usb_string_manufacturer_name_default = {
        2 + MANUFACTURER_NAME_LEN * 2,
        3,
        MANUFACTURER_NAME
};
struct usb_string_descriptor_struct usb_string_product_name_default = {
	2 + PRODUCT_NAME_LEN * 2,
        3,
        PRODUCT_NAME
};
struct usb_string_descriptor_struct usb_string_serial_number_default = {
        12,
        3,
        {0,0,0,0,0,0,0,0,0,0}
};


char * ultoa(unsigned long val, char *buf, int radix) 	
{
	unsigned digit;
	int i=0, j;
	char t;

	while (1) {
		digit = val % radix;
		buf[i] = ((digit < 10) ? '0' + digit : 'A' + digit - 10);
		val /= radix;
		if (val == 0) break;
		i++;
	}
	buf[i + 1] = 0;
	for (j=0; j < i; j++, i--) {
		t = buf[j];
		buf[j] = buf[i];
		buf[i] = t;
	}
	return buf;
}


void usb_init_serialnumber(void)
{
	char buf[11];
	uint32_t i, num;

	__disable_irq();
	FTFL_FSTAT = FTFL_FSTAT_RDCOLERR | FTFL_FSTAT_ACCERR | FTFL_FSTAT_FPVIOL;
	FTFL_FCCOB0 = 0x41;
	FTFL_FCCOB1 = 15;
	FTFL_FSTAT = FTFL_FSTAT_CCIF;
	while (!(FTFL_FSTAT & FTFL_FSTAT_CCIF)) ; // wait
	num = *(uint32_t *)&FTFL_FCCOB7;
	__enable_irq();
	// add extra zero to work around OS-X CDC-ACM driver bug
	if (num < 10000000) num = num * 10;
	ultoa(num, buf, 10);
	for (i=0; i<10; i++) {
		char c = buf[i];
		if (!c) break;
		usb_string_serial_number_default.wString[i] = c;
	}
	usb_string_serial_number_default.bLength = i * 2 + 2;
}


// **************************************************************
//   Descriptors List
// **************************************************************

// This table provides access to all the descriptor data above.

const usb_descriptor_list_t usb_descriptor_list[] = {
	//wValue, wIndex, address,          length
	{0x0100, 0x0000, device_descriptor, sizeof(device_descriptor)},
	{0x0200, 0x0000, config_descriptor, sizeof(config_descriptor)},
        {0x0300, 0x0000, (const uint8_t *)&string0, 0},
        {0x0301, 0x0409, (const uint8_t *)&usb_string_manufacturer_name, 0},
        {0x0302, 0x0409, (const uint8_t *)&usb_string_product_name, 0},
        {0x0303, 0x0409, (const uint8_t *)&usb_string_serial_number, 0},
	{0, 0, NULL, 0}
};


// **************************************************************
//   Endpoint Configuration
// **************************************************************



const uint8_t usb_endpoint_config_table[NUM_ENDPOINTS] =
{
#if (defined(ENDPOINT1_CONFIG) && NUM_ENDPOINTS >= 1)
	ENDPOINT1_CONFIG,
#elif (NUM_ENDPOINTS >= 1)
	ENDPOINT_UNUSED,
#endif
#if (defined(ENDPOINT2_CONFIG) && NUM_ENDPOINTS >= 2)
	ENDPOINT2_CONFIG,
#elif (NUM_ENDPOINTS >= 2)
	ENDPOINT_UNUSED,
#endif
#if (defined(ENDPOINT3_CONFIG) && NUM_ENDPOINTS >= 3)
	ENDPOINT3_CONFIG,
#elif (NUM_ENDPOINTS >= 3)
	ENDPOINT_UNUSED,
#endif
#if (defined(ENDPOINT4_CONFIG) && NUM_ENDPOINTS >= 4)
	ENDPOINT4_CONFIG,
#elif (NUM_ENDPOINTS >= 4)
	ENDPOINT_UNUSED,
#endif
};



/*
********************************************************************
 "usb_mem.c"
********************************************************************
*/


typedef struct usb_packet_struct {
	uint16_t len;
	uint16_t index;
	struct usb_packet_struct *next;
	uint8_t buf[64];
} usb_packet_t;

__attribute__ ((section(".usbbuffers"), used))
unsigned char usb_buffer_memory[NUM_USB_BUFFERS * sizeof(usb_packet_t)];

static uint32_t usb_buffer_available = 0xFFFFFFFF;

// use bitmask and CLZ instruction to implement fast free list
// http://www.archivum.info/gnu.gcc.help/2006-08/00148/Re-GCC-Inline-Assembly.html
// http://gcc.gnu.org/ml/gcc/2012-06/msg00015.html
// __builtin_clz()

usb_packet_t * usb_malloc(void)
{
	unsigned int n, avail;
	uint8_t *p;

	__disable_irq();
	avail = usb_buffer_available;
	n = __builtin_clz(avail); // clz = count leading zeros
	if (n >= NUM_USB_BUFFERS) {
		__enable_irq();
		return NULL;
	}
	usb_buffer_available = avail & ~(0x80000000 >> n);
	__enable_irq();
	p = usb_buffer_memory + (n * sizeof(usb_packet_t));
	*(uint32_t *)p = 0;
	*(uint32_t *)(p + 4) = 0;
	return (usb_packet_t *)p;
}

// for the receive endpoints to request memory
extern uint8_t usb_rx_memory_needed;
extern void usb_rx_memory(usb_packet_t *packet);

void usb_free(usb_packet_t *p)
{
	unsigned int n, mask;

	n = ((uint8_t *)p - usb_buffer_memory) / sizeof(usb_packet_t);
	if (n >= NUM_USB_BUFFERS) return;
	// if any endpoints are starving for memory to receive
	// packets, give this memory to them immediately!
	if (usb_rx_memory_needed && usb_configuration) {
		usb_rx_memory(p);
		return;
	}

	mask = (0x80000000 >> n);
	__disable_irq();
	usb_buffer_available |= mask;
	__enable_irq();
}

/*
********************************************************************
 "usb_dev.c"
********************************************************************
*/

// forward (originally in "usb_serial.c")
void usb_serial_flush_callback(void);

// buffer descriptor table
typedef struct {
	uint32_t desc;
	void * addr;
} bdt_t;

__attribute__ ((section(".usbdescriptortable"), used))
static bdt_t table[(NUM_ENDPOINTS+1)*4];

static usb_packet_t *rx_first[NUM_ENDPOINTS];
static usb_packet_t *rx_last[NUM_ENDPOINTS];
static usb_packet_t *tx_first[NUM_ENDPOINTS];
static usb_packet_t *tx_last[NUM_ENDPOINTS];
uint16_t usb_rx_byte_count_data[NUM_ENDPOINTS];

static uint8_t tx_state[NUM_ENDPOINTS];
#define TX_STATE_BOTH_FREE_EVEN_FIRST	0
#define TX_STATE_BOTH_FREE_ODD_FIRST	1
#define TX_STATE_EVEN_FREE		2
#define TX_STATE_ODD_FREE		3
#define TX_STATE_NONE_FREE_EVEN_FIRST	4
#define TX_STATE_NONE_FREE_ODD_FIRST	5

#define BDT_OWN		0x80
#define BDT_DATA1	0x40
#define BDT_DATA0	0x00
#define BDT_DTS		0x08
#define BDT_STALL	0x04
#define BDT_PID(n)	(((n) >> 2) & 15)

#define BDT_DESC(count, data)	(BDT_OWN | BDT_DTS \
				| ((data) ? BDT_DATA1 : BDT_DATA0) \
				| ((count) << 16))

#define TX   1
#define RX   0
#define ODD  1
#define EVEN 0
#define DATA0 0
#define DATA1 1
#define index(endpoint, tx, odd) (((endpoint) << 2) | ((tx) << 1) | (odd))
#define stat2bufferdescriptor(stat) (table + ((stat) >> 2))


static union {
 struct {
  union {
   struct {
	uint8_t bmRequestType;
	uint8_t bRequest;
   };
	uint16_t wRequestAndType;
  };
	uint16_t wValue;
	uint16_t wIndex;
	uint16_t wLength;
 };
 struct {
	uint32_t word1;
	uint32_t word2;
 };
} setup;


#define GET_STATUS		0
#define CLEAR_FEATURE		1
#define SET_FEATURE		3
#define SET_ADDRESS		5
#define GET_DESCRIPTOR		6
#define SET_DESCRIPTOR		7
#define GET_CONFIGURATION	8
#define SET_CONFIGURATION	9
#define GET_INTERFACE		10
#define SET_INTERFACE		11
#define SYNCH_FRAME		12

// SETUP always uses a DATA0 PID for the data field of the SETUP transaction.
// transactions in the data phase start with DATA1 and toggle (figure 8-12, USB1.1)
// Status stage uses a DATA1 PID.

static uint8_t ep0_rx0_buf[EP0_SIZE] __attribute__ ((aligned (4)));
static uint8_t ep0_rx1_buf[EP0_SIZE] __attribute__ ((aligned (4)));
static const uint8_t *ep0_tx_ptr = NULL;
static uint16_t ep0_tx_len;
static uint8_t ep0_tx_bdt_bank = 0;
static uint8_t ep0_tx_data_toggle = 0;
uint8_t usb_rx_memory_needed = 0;

volatile uint8_t usb_configuration = 0;
volatile uint8_t usb_reboot_timer = 0;


static void endpoint0_stall(void)
{
	USB0_ENDPT0 = USB_ENDPT_EPSTALL | USB_ENDPT_EPRXEN | USB_ENDPT_EPTXEN | USB_ENDPT_EPHSHK;
}


static void endpoint0_transmit(const void *data, uint32_t len)
{
	table[index(0, TX, ep0_tx_bdt_bank)].addr = (void *)data;
	table[index(0, TX, ep0_tx_bdt_bank)].desc = BDT_DESC(len, ep0_tx_data_toggle);
	ep0_tx_data_toggle ^= 1;
	ep0_tx_bdt_bank ^= 1;
}

static uint8_t reply_buffer[8];

static void usb_setup(void)
{
	const uint8_t *data = NULL;
	uint32_t datalen = 0;
	const usb_descriptor_list_t *list;
	uint32_t size;
	volatile uint8_t *reg;
	uint8_t epconf;
	const uint8_t *cfg;
	int i;

	switch (setup.wRequestAndType) {
	  case 0x0500: // SET_ADDRESS
		break;
	  case 0x0900: // SET_CONFIGURATION
		//serial_print("configure\n");
		usb_configuration = setup.wValue;
		reg = &USB0_ENDPT1;
		cfg = usb_endpoint_config_table;
		// clear all BDT entries, free any allocated memory...
		for (i=4; i < (NUM_ENDPOINTS+1)*4; i++) {
			if (table[i].desc & BDT_OWN) {
				usb_free((usb_packet_t *)((uint8_t *)(table[i].addr) - 8));
			}
		}
		// free all queued packets
		for (i=0; i < NUM_ENDPOINTS; i++) {
			usb_packet_t *p, *n;
			p = rx_first[i];
			while (p) {
				n = p->next;
				usb_free(p);
				p = n;
			}
			rx_first[i] = NULL;
			rx_last[i] = NULL;
			p = tx_first[i];
			while (p) {
				n = p->next;
				usb_free(p);
				p = n;
			}
			tx_first[i] = NULL;
			tx_last[i] = NULL;
			usb_rx_byte_count_data[i] = 0;
			switch (tx_state[i]) {
			  case TX_STATE_EVEN_FREE:
			  case TX_STATE_NONE_FREE_EVEN_FIRST:
				tx_state[i] = TX_STATE_BOTH_FREE_EVEN_FIRST;
				break;
			  case TX_STATE_ODD_FREE:
			  case TX_STATE_NONE_FREE_ODD_FIRST:
				tx_state[i] = TX_STATE_BOTH_FREE_ODD_FIRST;
				break;
			  default:
				break;
			}
		}
		usb_rx_memory_needed = 0;
		for (i=1; i <= NUM_ENDPOINTS; i++) {
			epconf = *cfg++;
			*reg = epconf;
			reg += 4;
			if (epconf & USB_ENDPT_EPRXEN) {
				usb_packet_t *p;
				p = usb_malloc();
				if (p) {
					table[index(i, RX, EVEN)].addr = p->buf;
					table[index(i, RX, EVEN)].desc = BDT_DESC(64, 0);
				} else {
					table[index(i, RX, EVEN)].desc = 0;
					usb_rx_memory_needed++;
				}
				p = usb_malloc();
				if (p) {
					table[index(i, RX, ODD)].addr = p->buf;
					table[index(i, RX, ODD)].desc = BDT_DESC(64, 1);
				} else {
					table[index(i, RX, ODD)].desc = 0;
					usb_rx_memory_needed++;
				}
			}
			table[index(i, TX, EVEN)].desc = 0;
			table[index(i, TX, ODD)].desc = 0;
		}
		break;
	  case 0x0880: // GET_CONFIGURATION
		reply_buffer[0] = usb_configuration;
		datalen = 1;
		data = reply_buffer;
		break;
	  case 0x0080: // GET_STATUS (device)
		reply_buffer[0] = 0;
		reply_buffer[1] = 0;
		datalen = 2;
		data = reply_buffer;
		break;
	  case 0x0082: // GET_STATUS (endpoint)
		if (setup.wIndex > NUM_ENDPOINTS) {
			// TODO: do we need to handle IN vs OUT here?
			endpoint0_stall();
			return;
		}
		reply_buffer[0] = 0;
		reply_buffer[1] = 0;
		if (*(uint8_t *)(&USB0_ENDPT0 + setup.wIndex * 4) & 0x02) reply_buffer[0] = 1;
		data = reply_buffer;
		datalen = 2;
		break;
	  case 0x0102: // CLEAR_FEATURE (endpoint)
		i = setup.wIndex & 0x7F;
		if (i > NUM_ENDPOINTS || setup.wValue != 0) {
			// TODO: do we need to handle IN vs OUT here?
			endpoint0_stall();
			return;
		}
		(*(uint8_t *)(&USB0_ENDPT0 + i * 4)) &= ~0x02;
		// TODO: do we need to clear the data toggle here?
		break;
	  case 0x0302: // SET_FEATURE (endpoint)
		i = setup.wIndex & 0x7F;
		if (i > NUM_ENDPOINTS || setup.wValue != 0) {
			// TODO: do we need to handle IN vs OUT here?
			endpoint0_stall();
			return;
		}
		(*(uint8_t *)(&USB0_ENDPT0 + i * 4)) |= 0x02;
		// TODO: do we need to clear the data toggle here?
		break;
	  case 0x0680: // GET_DESCRIPTOR
	  case 0x0681:
		//serial_print("desc:");
		//serial_phex16(setup.wValue);
		//serial_print("\n");
		for (list = usb_descriptor_list; 1; list++) {
			if (list->addr == NULL) break;
			//if (setup.wValue == list->wValue &&
			//(setup.wIndex == list->wIndex) || ((setup.wValue >> 8) == 3)) {
			if (setup.wValue == list->wValue && setup.wIndex == list->wIndex) {
				data = list->addr;
				if ((setup.wValue >> 8) == 3) {
					// for string descriptors, use the descriptor's
					// length field, allowing runtime configured
					// length.
					datalen = *(list->addr);
				} else {
					datalen = list->length;
				}
				goto send;
			}
		}
		//serial_print("desc: not found\n");
		endpoint0_stall();
		return;
#if defined(CDC_STATUS_INTERFACE)
	  case 0x2221: // CDC_SET_CONTROL_LINE_STATE
	       	usb_cdc_line_rtsdtr = setup.wValue;
		//serial_print("set control line state\n");
		break;
	  case 0x2321: // CDC_SEND_BREAK
		break;
	  case 0x2021: // CDC_SET_LINE_CODING
		//serial_print("set coding, waiting...\n");
		return;
#endif

	  default:
		endpoint0_stall();
		return;
	}
	send:

	if (datalen > setup.wLength) datalen = setup.wLength;
	size = datalen;
	if (size > EP0_SIZE) size = EP0_SIZE;
	endpoint0_transmit(data, size);
	data += size;
	datalen -= size;
	if (datalen == 0 && size < EP0_SIZE) return;

	size = datalen;
	if (size > EP0_SIZE) size = EP0_SIZE;
	endpoint0_transmit(data, size);
	data += size;
	datalen -= size;
	if (datalen == 0 && size < EP0_SIZE) return;

	ep0_tx_ptr = data;
	ep0_tx_len = datalen;
}



//A bulk endpoint's toggle sequence is initialized to DATA0 when the endpoint
//experiences any configuration event (configuration events are explained in
//Sections 9.1.1.5 and 9.4.5).

//Configuring a device or changing an alternate setting causes all of the status
//and configuration values associated with endpoints in the affected interfaces
//to be set to their default values. This includes setting the data toggle of
//any endpoint using data toggles to the value DATA0.

//For endpoints using data toggle, regardless of whether an endpoint has the
//Halt feature set, a ClearFeature(ENDPOINT_HALT) request always results in the
//data toggle being reinitialized to DATA0.



// #define stat2bufferdescriptor(stat) (table + ((stat) >> 2))

static void usb_control(uint32_t stat)
{
	bdt_t *b;
	uint32_t pid, size;
	uint8_t *buf;
	const uint8_t *data;

	b = stat2bufferdescriptor(stat);
	pid = BDT_PID(b->desc);
	buf = b->addr;

	switch (pid) {
	case 0x0D: // Setup received from host
		//serial_print("PID=Setup\n");
		//if (count != 8) ; // panic?
		// grab the 8 byte setup info
		setup.word1 = *(uint32_t *)(buf);
		setup.word2 = *(uint32_t *)(buf + 4);

		// give the buffer back
		b->desc = BDT_DESC(EP0_SIZE, DATA1);
		//table[index(0, RX, EVEN)].desc = BDT_DESC(EP0_SIZE, 1);
		//table[index(0, RX, ODD)].desc = BDT_DESC(EP0_SIZE, 1);

		// clear any leftover pending IN transactions
		ep0_tx_ptr = NULL;
		if (ep0_tx_data_toggle) {
		}
		//if (table[index(0, TX, EVEN)].desc & 0x80) {
			//serial_print("leftover tx even\n");
		//}
		//if (table[index(0, TX, ODD)].desc & 0x80) {
			//serial_print("leftover tx odd\n");
		//}
		table[index(0, TX, EVEN)].desc = 0;
		table[index(0, TX, ODD)].desc = 0;
		// first IN after Setup is always DATA1
		ep0_tx_data_toggle = 1;
		// actually "do" the setup request
		usb_setup();
		// unfreeze the USB, now that we're ready
		USB0_CTL = USB_CTL_USBENSOFEN; // clear TXSUSPENDTOKENBUSY bit
		break;
	case 0x01:  // OUT transaction received from host
	case 0x02:
		//serial_print("PID=OUT\n");
#ifdef CDC_STATUS_INTERFACE
		if (setup.wRequestAndType == 0x2021 /*CDC_SET_LINE_CODING*/) {
			int i;
			uint8_t *dst = (uint8_t *)usb_cdc_line_coding;
			//serial_print("set line coding ");
			for (i=0; i<7; i++) {
				//serial_phex(*buf);
				*dst++ = *buf++;
			}
			//serial_phex32(usb_cdc_line_coding[0]);
			//serial_print("\n");
			if (usb_cdc_line_coding[0] == 134) usb_reboot_timer = 15;
			endpoint0_transmit(NULL, 0);
		}
#endif
		// give the buffer back
		b->desc = BDT_DESC(EP0_SIZE, DATA1);
		break;

	case 0x09: // IN transaction completed to host
		//serial_print("PID=IN:");
		//serial_phex(stat);
		//serial_print("\n");

		// send remaining data, if any...
		data = ep0_tx_ptr;
		if (data) {
			size = ep0_tx_len;
			if (size > EP0_SIZE) size = EP0_SIZE;
			endpoint0_transmit(data, size);
			data += size;
			ep0_tx_len -= size;
			ep0_tx_ptr = (ep0_tx_len > 0 || size == EP0_SIZE) ? data : NULL;
		}

		if (setup.bRequest == 5 && setup.bmRequestType == 0) {
			setup.bRequest = 0;
			//serial_print("set address: ");
			//serial_phex16(setup.wValue);
			//serial_print("\n");
			USB0_ADDR = setup.wValue;
		}

		break;
	//default:
		//serial_print("PID=unknown:");
		//serial_phex(pid);
		//serial_print("\n");
	}
	USB0_CTL = USB_CTL_USBENSOFEN; // clear TXSUSPENDTOKENBUSY bit
}






usb_packet_t *usb_rx(uint32_t endpoint)
{
	usb_packet_t *ret;
	endpoint--;
	if (endpoint >= NUM_ENDPOINTS) return NULL;
	__disable_irq();
	ret = rx_first[endpoint];
	if (ret) {
		rx_first[endpoint] = ret->next;
		usb_rx_byte_count_data[endpoint] -= ret->len;
	}
	__enable_irq();
	//serial_print("rx, epidx=");
	//serial_phex(endpoint);
	//serial_print(", packet=");
	//serial_phex32(ret);
	//serial_print("\n");
	return ret;
}

static uint32_t usb_queue_byte_count(const usb_packet_t *p)
{
	uint32_t count=0;

	__disable_irq();
	for ( ; p; p = p->next) {
		count += p->len;
	}
	__enable_irq();
	return count;
}

static inline uint32_t usb_rx_byte_count(uint32_t endpoint) __attribute__((always_inline));
static inline uint32_t usb_rx_byte_count(uint32_t endpoint)
{
        endpoint--;
        if (endpoint >= NUM_ENDPOINTS) return 0;
        return usb_rx_byte_count_data[endpoint];
}

uint32_t usb_tx_byte_count(uint32_t endpoint)
{
	endpoint--;
	if (endpoint >= NUM_ENDPOINTS) return 0;
	return usb_queue_byte_count(tx_first[endpoint]);
}

uint32_t usb_tx_packet_count(uint32_t endpoint)
{
	const usb_packet_t *p;
	uint32_t count=0;

	endpoint--;
	if (endpoint >= NUM_ENDPOINTS) return 0;
	__disable_irq();
	for (p = tx_first[endpoint]; p; p = p->next) count++;
	__enable_irq();
	return count;
}


// Called from usb_free, but only when usb_rx_memory_needed > 0, indicating
// receive endpoints are starving for memory.  The intention is to give
// endpoints needing receive memory priority over the user's code, which is
// likely calling usb_malloc to obtain memory for transmitting.  When the
// user is creating data very quickly, their consumption could starve reception
// without this prioritization.  The packet buffer (input) is assigned to the
// first endpoint needing memory.
//
void usb_rx_memory(usb_packet_t *packet)
{
	unsigned int i;
	const uint8_t *cfg;

	cfg = usb_endpoint_config_table;
	//serial_print("rx_mem:");
	__disable_irq();
	for (i=1; i <= NUM_ENDPOINTS; i++) {
		if (*cfg++ & USB_ENDPT_EPRXEN) {
			if (table[index(i, RX, EVEN)].desc == 0) {
				table[index(i, RX, EVEN)].addr = packet->buf;
				table[index(i, RX, EVEN)].desc = BDT_DESC(64, 0);
				usb_rx_memory_needed--;
				__enable_irq();
				//serial_phex(i);
				//serial_print(",even\n");
				return;
			}
			if (table[index(i, RX, ODD)].desc == 0) {
				table[index(i, RX, ODD)].addr = packet->buf;
				table[index(i, RX, ODD)].desc = BDT_DESC(64, 1);
				usb_rx_memory_needed--;
				__enable_irq();
				//serial_phex(i);
				//serial_print(",odd\n");
				return;
			}
		}
	}
	__enable_irq();
	// we should never reach this point.  If we get here, it means
	// usb_rx_memory_needed was set greater than zero, but no memory
	// was actually needed.
	usb_rx_memory_needed = 0;
	usb_free(packet);
	return;
}

//#define index(endpoint, tx, odd) (((endpoint) << 2) | ((tx) << 1) | (odd))
//#define stat2bufferdescriptor(stat) (table + ((stat) >> 2))

void usb_tx(uint32_t endpoint, usb_packet_t *packet)
{
	bdt_t *b = &table[index(endpoint, TX, EVEN)];
	uint8_t next;

	endpoint--;
	if (endpoint >= NUM_ENDPOINTS) return;
	__disable_irq();
	//serial_print("txstate=");
	//serial_phex(tx_state[endpoint]);
	//serial_print("\n");
	switch (tx_state[endpoint]) {
	  case TX_STATE_BOTH_FREE_EVEN_FIRST:
		next = TX_STATE_ODD_FREE;
		break;
	  case TX_STATE_BOTH_FREE_ODD_FIRST:
		b++;
		next = TX_STATE_EVEN_FREE;
		break;
	  case TX_STATE_EVEN_FREE:
		next = TX_STATE_NONE_FREE_ODD_FIRST;
		break;
	  case TX_STATE_ODD_FREE:
		b++;
		next = TX_STATE_NONE_FREE_EVEN_FIRST;
		break;
	  default:
		if (tx_first[endpoint] == NULL) {
			tx_first[endpoint] = packet;
		} else {
			tx_last[endpoint]->next = packet;
		}
		tx_last[endpoint] = packet;
		__enable_irq();
		return;
	}
	tx_state[endpoint] = next;
	b->addr = packet->buf;
	b->desc = BDT_DESC(packet->len, ((uint32_t)b & 8) ? DATA1 : DATA0);
	__enable_irq();
}






void _reboot_Teensyduino_(void)
{
	// TODO: initialize R0 with a code....
	__asm__ volatile("bkpt");
}



void usb_isr(void)
{
	uint8_t status, stat, t;

	restart:
	status = USB0_ISTAT;

	if ((status & USB_ISTAT_SOFTOK /* 04 */ )) {
		if (usb_configuration) {
			t = usb_reboot_timer;
			if (t) {
				usb_reboot_timer = --t;
				if (!t) _reboot_Teensyduino_();
			}
#ifdef CDC_DATA_INTERFACE
			t = usb_cdc_transmit_flush_timer;
			if (t) {
				usb_cdc_transmit_flush_timer = --t;
				if (t == 0) usb_serial_flush_callback();
			}
#endif
		}
		USB0_ISTAT = USB_ISTAT_SOFTOK;
	}

	if ((status & USB_ISTAT_TOKDNE /* 08 */ )) {
		uint8_t endpoint;
		stat = USB0_STAT;
		endpoint = stat >> 4;
		if (endpoint == 0) {
			usb_control(stat);
		} else {
			bdt_t *b = stat2bufferdescriptor(stat);
			usb_packet_t *packet = (usb_packet_t *)((uint8_t *)(b->addr) - 8);

			endpoint--;	// endpoint is index to zero-based arrays

			if (stat & 0x08) { // transmit
				usb_free(packet);
				packet = tx_first[endpoint];
				if (packet) {
					//serial_print("tx packet\n");
					tx_first[endpoint] = packet->next;
					b->addr = packet->buf;
					switch (tx_state[endpoint]) {
					  case TX_STATE_BOTH_FREE_EVEN_FIRST:
						tx_state[endpoint] = TX_STATE_ODD_FREE;
						break;
					  case TX_STATE_BOTH_FREE_ODD_FIRST:
						tx_state[endpoint] = TX_STATE_EVEN_FREE;
						break;
					  case TX_STATE_EVEN_FREE:
						tx_state[endpoint] = TX_STATE_NONE_FREE_ODD_FIRST;
						break;
					  case TX_STATE_ODD_FREE:
						tx_state[endpoint] = TX_STATE_NONE_FREE_EVEN_FIRST;
						break;
					  default:
						break;
					}
					b->desc = BDT_DESC(packet->len, ((uint32_t)b & 8) ? DATA1 : DATA0);
				} else {
					//serial_print("tx no packet\n");
					switch (tx_state[endpoint]) {
					  case TX_STATE_BOTH_FREE_EVEN_FIRST:
					  case TX_STATE_BOTH_FREE_ODD_FIRST:
						break;
					  case TX_STATE_EVEN_FREE:
						tx_state[endpoint] = TX_STATE_BOTH_FREE_EVEN_FIRST;
						break;
					  case TX_STATE_ODD_FREE:
						tx_state[endpoint] = TX_STATE_BOTH_FREE_ODD_FIRST;
						break;
					  default:
						tx_state[endpoint] = ((uint32_t)b & 8) ?
						  TX_STATE_ODD_FREE : TX_STATE_EVEN_FREE;
						break;
					}
				}
			} else { // receive
				packet->len = b->desc >> 16;
				if (packet->len > 0) {
					packet->index = 0;
					packet->next = NULL;
					if (rx_first[endpoint] == NULL) {
						//serial_print("rx 1st, epidx=");
						//serial_phex(endpoint);
						//serial_print(", packet=");
						//serial_phex32((uint32_t)packet);
						//serial_print("\n");
						rx_first[endpoint] = packet;
					} else {
						//serial_print("rx Nth, epidx=");
						//serial_phex(endpoint);
						//serial_print(", packet=");
						//serial_phex32((uint32_t)packet);
						//serial_print("\n");
						rx_last[endpoint]->next = packet;
					}
					rx_last[endpoint] = packet;
					usb_rx_byte_count_data[endpoint] += packet->len;
					// TODO: implement a per-endpoint maximum # of allocated packets
					// so a flood of incoming data on 1 endpoint doesn't starve
					// the others if the user isn't reading it regularly
					packet = usb_malloc();
					if (packet) {
						b->addr = packet->buf;
						b->desc = BDT_DESC(64, ((uint32_t)b & 8) ? DATA1 : DATA0);
					} else {
						//serial_print("starving ");
						//serial_phex(endpoint + 1);
						//serial_print(((uint32_t)b & 8) ? ",odd\n" : ",even\n");
						b->desc = 0;
						usb_rx_memory_needed++;
					}
				} else {
					b->desc = BDT_DESC(64, ((uint32_t)b & 8) ? DATA1 : DATA0);
				}
			}




		}
		USB0_ISTAT = USB_ISTAT_TOKDNE;
		goto restart;
	}



	if (status & USB_ISTAT_USBRST /* 01 */ ) {
		//serial_print("reset\n");

		// initialize BDT toggle bits
		USB0_CTL = USB_CTL_ODDRST;
		ep0_tx_bdt_bank = 0;

		// set up buffers to receive Setup and OUT packets
		table[index(0, RX, EVEN)].desc = BDT_DESC(EP0_SIZE, 0);
		table[index(0, RX, EVEN)].addr = ep0_rx0_buf;
		table[index(0, RX, ODD)].desc = BDT_DESC(EP0_SIZE, 0);
		table[index(0, RX, ODD)].addr = ep0_rx1_buf;
		table[index(0, TX, EVEN)].desc = 0;
		table[index(0, TX, ODD)].desc = 0;

		// activate endpoint 0
		USB0_ENDPT0 = USB_ENDPT_EPRXEN | USB_ENDPT_EPTXEN | USB_ENDPT_EPHSHK;

		// clear all ending interrupts
		USB0_ERRSTAT = 0xFF;
		USB0_ISTAT = 0xFF;

		// set the address to zero during enumeration
		USB0_ADDR = 0;

		// enable other interrupts
		USB0_ERREN = 0xFF;
		USB0_INTEN = USB_INTEN_TOKDNEEN |
			USB_INTEN_SOFTOKEN |
			USB_INTEN_STALLEN |
			USB_INTEN_ERROREN |
			USB_INTEN_USBRSTEN |
			USB_INTEN_SLEEPEN;

		// is this necessary?
		USB0_CTL = USB_CTL_USBENSOFEN;
		return;
	}


	if ((status & USB_ISTAT_STALL /* 80 */ )) {
		//serial_print("stall:\n");
		USB0_ENDPT0 = USB_ENDPT_EPRXEN | USB_ENDPT_EPTXEN | USB_ENDPT_EPHSHK;
		USB0_ISTAT = USB_ISTAT_STALL;
	}
	if ((status & USB_ISTAT_ERROR /* 02 */ )) {
		uint8_t err = USB0_ERRSTAT;
		USB0_ERRSTAT = err;
		//serial_print("err:");
		//serial_phex(err);
		//serial_print("\n");
		USB0_ISTAT = USB_ISTAT_ERROR;
	}

	if ((status & USB_ISTAT_SLEEP /* 10 */ )) {
		//serial_print("sleep\n");
		USB0_ISTAT = USB_ISTAT_SLEEP;
	}

}



void usb_init(void)
{
	int i;

	//serial_begin(BAUD2DIV(115200));
	//serial_print("usb_init\n");

	usb_init_serialnumber();

	for (i=0; i <= NUM_ENDPOINTS*4; i++) {
		table[i].desc = 0;
		table[i].addr = 0;
	}

	// this basically follows the flowchart in the Kinetis
	// Quick Reference User Guide, Rev. 1, 03/2012, page 141

	// assume 48 MHz clock already running
	// SIM - enable clock
	SIM_SCGC4 |= SIM_SCGC4_USBOTG;
#ifdef HAS_KINETIS_MPU
	MPU_RGDAAC0 |= 0x03000000;
#endif

	// reset USB module
	//USB0_USBTRC0 = USB_USBTRC_USBRESET;
	//while ((USB0_USBTRC0 & USB_USBTRC_USBRESET) != 0) ; // wait for reset to end

	// set desc table base addr
	USB0_BDTPAGE1 = ((uint32_t)table) >> 8;
	USB0_BDTPAGE2 = ((uint32_t)table) >> 16;
	USB0_BDTPAGE3 = ((uint32_t)table) >> 24;

	// clear all ISR flags
	USB0_ISTAT = 0xFF;
	USB0_ERRSTAT = 0xFF;
	USB0_OTGISTAT = 0xFF;

	//USB0_USBTRC0 |= 0x40; // undocumented bit

	// enable USB
	USB0_CTL = USB_CTL_USBENSOFEN;
	USB0_USBCTRL = 0;

	// enable reset interrupt
	USB0_INTEN = USB_INTEN_USBRSTEN;

	// enable interrupt in NVIC...
	NVIC_SET_PRIORITY(IRQ_USBOTG, 112);
	NVIC_ENABLE_IRQ(IRQ_USBOTG);

	// enable d+ pullup
	USB0_CONTROL = USB_CONTROL_DPPULLUPNONOTG;
}



/*
********************************************************************
 "usb_serial.c"
********************************************************************
*/


uint32_t usb_cdc_line_coding[2];
volatile uint8_t usb_cdc_line_rtsdtr=0;
volatile uint8_t usb_cdc_transmit_flush_timer=0;

static usb_packet_t *rx_packet=NULL;
static usb_packet_t *tx_packet=NULL;
static volatile uint8_t tx_noautoflush=0;

#define TRANSMIT_FLUSH_TIMEOUT	5   /* in milliseconds */

// get the next character, or -1 if nothing received
int usb_serial_getchar(void)
{
	unsigned int i;
	int c;

	if (!rx_packet) {
		if (!usb_configuration) return -1;
		rx_packet = usb_rx(CDC_RX_ENDPOINT);
		if (!rx_packet) return -1;
	}
	i = rx_packet->index;
	c = rx_packet->buf[i++];
	if (i >= rx_packet->len) {
		usb_free(rx_packet);
		rx_packet = NULL;
	} else {
		rx_packet->index = i;
	}
	return c;
}

// peek at the next character, or -1 if nothing received
int usb_serial_peekchar(void)
{
	if (!rx_packet) {
		if (!usb_configuration) return -1;
		rx_packet = usb_rx(CDC_RX_ENDPOINT);
		if (!rx_packet) return -1;
	}
	if (!rx_packet) return -1;
	return rx_packet->buf[rx_packet->index];
}

// number of bytes available in the receive buffer
int usb_serial_available(void)
{
	int count;
	count = usb_rx_byte_count(CDC_RX_ENDPOINT);
	if (rx_packet) count += rx_packet->len - rx_packet->index;
	return count;
}

// read a block of bytes to a buffer
int usb_serial_read(void *buffer, uint32_t size)
{
	uint8_t *p = (uint8_t *)buffer;
	uint32_t qty, count=0;

	while (size) {
		if (!usb_configuration) break;
		if (!rx_packet) {
			rx:
			rx_packet = usb_rx(CDC_RX_ENDPOINT);
			if (!rx_packet) break;
			if (rx_packet->len == 0) {
				usb_free(rx_packet);
				goto rx;
			}
		}
		qty = rx_packet->len - rx_packet->index;
		if (qty > size) qty = size;
		memcpy(p, rx_packet->buf + rx_packet->index, qty);
		p += qty;
		count += qty;
		size -= qty;
		rx_packet->index += qty;
		if (rx_packet->index >= rx_packet->len) {
			usb_free(rx_packet);
			rx_packet = NULL;
		}
	}
	return count;
}

// discard any buffered input
void usb_serial_flush_input(void)
{
	usb_packet_t *rx;

	if (!usb_configuration) return;
	if (rx_packet) {
		usb_free(rx_packet);
		rx_packet = NULL;
	}
	while (1) {
		rx = usb_rx(CDC_RX_ENDPOINT);
		if (!rx) break;
		usb_free(rx);
	}
}

// Maximum number of transmit packets to queue so we don't starve other endpoints for memory
#define TX_PACKET_LIMIT 8

// When the PC isn't listening, how long do we wait before discarding data?  If this is
// too short, we risk losing data during the stalls that are common with ordinary desktop
// software.  If it's too long, we stall the user's program when no software is running.
#define TX_TIMEOUT_MSEC 70

#if F_CPU == 96000000
  #define TX_TIMEOUT (TX_TIMEOUT_MSEC * 596)
#elif F_CPU == 72000000
  #define TX_TIMEOUT (TX_TIMEOUT_MSEC * 512)
#elif F_CPU == 48000000
  #define TX_TIMEOUT (TX_TIMEOUT_MSEC * 428)
#elif F_CPU == 24000000
  #define TX_TIMEOUT (TX_TIMEOUT_MSEC * 262)
#else
  #error Unsupported cpu clock
#endif

// When we've suffered the transmit timeout, don't wait again until the computer
// begins accepting data.  If no software is running to receive, we'll just discard
// data as rapidly as Serial.print() can generate it, until there's something to
// actually receive it.
static uint8_t transmit_previous_timeout=0;


// transmit a character.  0 returned on success, -1 on error
int usb_serial_putchar(uint8_t c)
{
	return usb_serial_write(&c, 1);
}


int usb_serial_write(const void *buffer, uint32_t size)
{
	uint32_t len;
	uint32_t wait_count;
	const uint8_t *src = (const uint8_t *)buffer;
	uint8_t *dest;

	tx_noautoflush = 1;
	while (size > 0) {
		if (!tx_packet) {
			wait_count = 0;
			while (1) {
				if (!usb_configuration) {
					tx_noautoflush = 0;
					return -1;
				}
				if (usb_tx_packet_count(CDC_TX_ENDPOINT) < TX_PACKET_LIMIT) {
					tx_noautoflush = 1;
					tx_packet = usb_malloc();
					if (tx_packet) break;
					tx_noautoflush = 0;
				}
				if (++wait_count > TX_TIMEOUT || transmit_previous_timeout) {
					transmit_previous_timeout = 1;
					return -1;
				}
				//	yield();
			}
		}
		transmit_previous_timeout = 0;
		len = CDC_TX_SIZE - tx_packet->index;
		if (len > size) len = size;
		dest = tx_packet->buf + tx_packet->index;
		tx_packet->index += len;
		size -= len;
		while (len-- > 0) *dest++ = *src++;
		if (tx_packet->index >= CDC_TX_SIZE) {
			tx_packet->len = CDC_TX_SIZE;
			usb_tx(CDC_TX_ENDPOINT, tx_packet);
			tx_packet = NULL;
		}
		usb_cdc_transmit_flush_timer = TRANSMIT_FLUSH_TIMEOUT;
	}
	tx_noautoflush = 0;
	return 0;
}

int usb_serial_write_buffer_free(void)
{
	uint32_t len;

	tx_noautoflush = 1;
	if (!tx_packet) {
		if (!usb_configuration ||
		  usb_tx_packet_count(CDC_TX_ENDPOINT) >= TX_PACKET_LIMIT ||
		  (tx_packet = usb_malloc()) == NULL) {
			tx_noautoflush = 0;
			return 0;
		}
	}
	len = CDC_TX_SIZE - tx_packet->index;
	// TODO: Perhaps we need "usb_cdc_transmit_flush_timer = TRANSMIT_FLUSH_TIMEOUT"
	// added here, so the SOF interrupt can't take away the available buffer
	// space we just promised the user could write without blocking?
	// But does this come with other performance downsides?  Could it lead to
	// buffer data never actually transmitting in some usage cases?  More
	// investigation is needed.
	// https://github.com/PaulStoffregen/cores/issues/10#issuecomment-61514955
	tx_noautoflush = 0;
	return len;
}

void usb_serial_flush_output(void)
{
	if (!usb_configuration) return;
	tx_noautoflush = 1;
	if (tx_packet) {
		usb_cdc_transmit_flush_timer = 0;
		tx_packet->len = tx_packet->index;
		usb_tx(CDC_TX_ENDPOINT, tx_packet);
		tx_packet = NULL;
	} else {
		usb_packet_t *tx = usb_malloc();
		if (tx) {
			usb_cdc_transmit_flush_timer = 0;
			usb_tx(CDC_TX_ENDPOINT, tx);
		} else {
			usb_cdc_transmit_flush_timer = 1;
		}
	}
	tx_noautoflush = 0;
}

void usb_serial_flush_callback(void)
{
	if (tx_noautoflush) return;
	if (tx_packet) {
		tx_packet->len = tx_packet->index;
		usb_tx(CDC_TX_ENDPOINT, tx_packet);
		tx_packet = NULL;
	} else {
		usb_packet_t *tx = usb_malloc();
		if (tx) {
			usb_tx(CDC_TX_ENDPOINT, tx);
		} else {
			usb_cdc_transmit_flush_timer = 1;
		}
	}
}


