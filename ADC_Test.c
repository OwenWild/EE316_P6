#include "xsysmon.h"
#include "xgpio.h"
#include "xparameters.h"
#include "xil_printf.h"

// Device IDs
#define XADC_DEVICE_ID XPAR_XADC_WIZ_0_DEVICE_ID
#define GPIO_DEVICE_ID XPAR_AXI_GPIO_0_DEVICE_ID

XSysMon Xadc;
XGpio Gpio;

int main() {

    XSysMon_Config *XadcConfig;
    int status;
    xil_printf("Hello");
    // Initialize XADC
    XadcConfig = XSysMon_LookupConfig(XADC_DEVICE_ID);
    XSysMon_CfgInitialize(&Xadc, XadcConfig, XadcConfig->BaseAddress);

    // Initialize GPIO
    status = XGpio_Initialize(&Gpio, GPIO_DEVICE_ID);
    if (status != XST_SUCCESS) {
        xil_printf("GPIO init failed\r\n");
        return -1;
    }

    // Set LEDs as outputs (channel 1)
    XGpio_SetDataDirection(&Gpio, 1, 0x0);

    xil_printf("PWM from potentiometer started...\r\n");

    int pwm_counter = 0;


    	while (1) {

    	    xil_printf("Loop running\r\n");

    	    u16 adc = XSysMon_GetAdcData(&Xadc, XSM_CH_AUX_MIN + 0);

    	    xil_printf("ADC: %d\r\n", adc);

    	    for (volatile int i = 0; i < 1000000; i++);
    	}

    return 0;
}


