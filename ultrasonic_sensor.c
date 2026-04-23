/******************************************************************************
*
* Copyright (C) 2018 Xilinx, Inc.  All rights reserved.
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
* XILINX  BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
* WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
* OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*
* Except as contained in this notice, the name of the Xilinx shall not be used
* in advertising or otherwise to promote the sale, use or other dealings in
* this Software without prior written authorization from Xilinx.
*
******************************************************************************/
/*****************************************************************************/
/**
* @file  xtmrctr_pwm_example.c
*
* This file contains a design example using the timer counter driver
* and hardware device using interrupt mode. The example demonstrates
* the use of PWM feature of axi timers. PWM is configured to operate at specific
* duty cycle and after every N cycles the duty cycle is incremented until a
* specific duty cycle is achieved. No software validation of duty cycle is
* undergone in the example.
*
* This example assumes that the interrupt controller is also present as a part
* of the system.
*
*
*
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date	 Changes
* ----- ---- -------- -----------------------------------------------
* 1.00b cjp  03/28/18 First release
*</pre>
******************************************************************************/

/***************************** Include Files *********************************/
#include "xtmrctr.h"
#include "xparameters.h"
#include "xil_exception.h"
#include "xil_io.h"
#include <stdio.h>

#ifdef XPAR_INTC_0_DEVICE_ID
#include "xintc.h"
#include <stdio.h>
#else
#include "xscugic.h"
#include "xil_printf.h"
#endif

/************************** Constant Definitions *****************************/
/*
 * The following constants map to the XPAR parameters created in the
 * xparameters.h file. They are only defined here such that a user can easily
 * change all the needed parameters in one place.
 */
#define TMRCTR_DEVICE_ID        XPAR_TMRCTR_0_DEVICE_ID
#define TMRCTR_1_DEVICE_ID        XPAR_TMRCTR_1_DEVICE_ID

#ifdef __MICROBLAZE__
#define TMRCTR_INTERRUPT_ID     XPAR_INTC_0_TMRCTR_0_VEC_ID
#else
#define TMRCTR_INTERRUPT_ID     XPAR_FABRIC_TMRCTR_0_VEC_ID
//#define TMRCTR_1_INTERRUPT_ID     XPAR_FABRIC_TMRCTR_1_VEC_ID
#endif

#ifdef XPAR_INTC_0_DEVICE_ID
#define INTC_DEVICE_ID          XPAR_INTC_0_DEVICE_ID
#define INTC                    XIntc
#define INTC_HANDLER            XIntc_InterruptHandler
#else
#define INTC_DEVICE_ID          XPAR_SCUGIC_SINGLE_DEVICE_ID
#define INTC                    XScuGic
#define INTC_HANDLER            XScuGic_InterruptHandler
#endif /* XPAR_INTC_0_DEVICE_ID */

#define PWM_PERIOD              20000000     /* PWM period in ns */
#define DUTY_PERIOD             15000          /* DUTY period in ns */
#define TMRCTR_0_0                0            /* Timer 0 ID */
#define TMRCTR_0_1                1            /* Timer 1 ID */
#define TMRCTR_1_0                0            /* Timer 0 ID */
#define TMRCTR_1_1                1            /* Timer 1 ID */
//#define CYCLE_PER_DUTYCYCLE     10           /* Clock cycles per duty cycle */
//#define MAX_DUTYCYCLE           100          /* Max duty cycle */
#define DUTYCYCLE_DIVISOR       4              /* Duty cycle Divisor */
#define WAIT_COUNT              50000000       /* Interrupt wait counter */

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/
int TmrCtrCapture(INTC *IntcInstancePtr, XTmrCtr *InstancePtr, u16 DeviceId,
								u16 IntrId);
static void TimerCounterHandler_0(void *CallBackRef, u8 TmrCtrNumber);
static void TimerCounterHandler_1(void *CallBackRef, u8 TmrCtrNumber);
static int TmrCtrSetupIntrSystem(INTC *IntcInstancePtr, XTmrCtr *InstancePtr,
						u16 DeviceId, u16 IntrId);
int TmrCtrPwmExample(INTC *IntcInstancePtr, XTmrCtr *InstancePtr, u16 DeviceId);
/************************** Variable Definitions *****************************/
INTC InterruptController;  /* The instance of the Interrupt Controller */
XTmrCtr TimerCounterInst;  /* The instance of the Timer Counter */

/*
 * The following variables are shared between non-interrupt processing and
 * interrupt processing such that they must be global.
 */
static int   PeriodTimerHit = FALSE;
static int   HighTimerHit = FALSE;
static u32   capture0, capture1, pulsewidth;
float distance;

/*****************************************************************************/
/**
* This function is the main function of the Tmrctr PWM example.
*
* @param	None.
*
* @return	XST_SUCCESS to indicate success, else XST_FAILURE to indicate a
*		Failure.
*
* @note		None.
*
******************************************************************************/
int main(void)
{
	//int i = 0;
	int Status;

	/* Run the Timer Counter PWM example */
	Status = TmrCtrPwmExample(&InterruptController, &TimerCounterInst,
				  TMRCTR_1_DEVICE_ID);
	if (Status != XST_SUCCESS) {
		xil_printf("Tmrctr PWM Example Failed\r\n");
		return XST_FAILURE;
	}


	/* Run the Timer Counter Capture example */
	Status = TmrCtrCapture(&InterruptController, &TimerCounterInst,
				  TMRCTR_DEVICE_ID, TMRCTR_INTERRUPT_ID);
	if (Status != XST_SUCCESS) {
		xil_printf("Tmrctr Capture Example Failed\r\n");
		return XST_FAILURE;
	}
	xil_printf("Successfully Started the Capture Example\r\n");

	while(1){

    distance = (float)pulsewidth*0.020/148;
//    printf("cap1 = %lu cap0 = %lu pulsewidth = %lu\r\n", capture1, capture0, pulsewidth);
    printf("Distance %5.2F inches \r\n", distance);
	}
	return XST_SUCCESS;
}


/*****************************************************************************/
/**
* This function demonstrates the use of tmrctr PWM APIs.
*
* @param	IntcInstancePtr is a pointer to the Interrupt Controller
*		driver Instance
* @param	TmrCtrInstancePtr is a pointer to the XTmrCtr driver Instance
* @param	DeviceId is the XPAR_<TmrCtr_instance>_DEVICE_ID value from
*		xparameters.h
* @param	IntrId is XPAR_<INTC_instance>_<TmrCtr_instance>_INTERRUPT_INTR
*		value from xparameters.h
*
* @return	XST_SUCCESS if the Test is successful, otherwise XST_FAILURE
*
* @note		none.
*
*****************************************************************************/
int TmrCtrPwmExample(INTC *IntcInstancePtr, XTmrCtr *TmrCtrInstancePtr,
						u16 DeviceId)
{
//	u8  DutyCycle;
//	float DutyCycle_percent;
//	u8  NoOfCycles;
//	u8  Div;
	u32 Period;
	u32 HighTime;
//	u64 WaitCount;
	int Status;

	/*
	 * Initialize the timer counter so that it's ready to use,
	 * specify the device ID that is generated in xparameters.h
	 */
	Status = XTmrCtr_Initialize(TmrCtrInstancePtr, DeviceId);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Perform a self-test to ensure that the hardware was built
	 * correctly. Timer0 is used for self test
	 */
	Status = XTmrCtr_SelfTest(TmrCtrInstancePtr, TMRCTR_1_0);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Connect the timer counter to the interrupt subsystem such that
	 * interrupts can occur
	 */
//	Status = TmrCtrSetupIntrSystem(IntcInstancePtr, TmrCtrInstancePtr,
//							DeviceId, IntrId);
//	if (Status != XST_SUCCESS) {
//		return XST_FAILURE;
//	}

	/*
	 * Setup the handler for the timer counter that will be called from the
	 * interrupt context when the timer expires
//	 */
//	XTmrCtr_SetHandler(TmrCtrInstancePtr, TimerCounterHandler_1,
//							TmrCtrInstancePtr);

//	/* Enable the interrupt of the timer counter */
//	XTmrCtr_SetOptions(TmrCtrInstancePtr, TMRCTR_1_0, XTC_INT_MODE_OPTION);
//	XTmrCtr_SetOptions(TmrCtrInstancePtr, TMRCTR_1_1, XTC_INT_MODE_OPTION);

	u32 pwmmasks = XTC_CSR_ENABLE_INT_MASK | XTC_CSR_DOWN_COUNT_MASK
			| XTC_CSR_AUTO_RELOAD_MASK;

	Xil_Out32(0x42810000, 0); //Turn off all fields.
	Xil_Out32(0x42810010, 0); //Turn off all fields.

	Xil_Out32(0x42810000, pwmmasks); //Start timer0 with count down/auto reload capability and interrupts.
	Xil_Out32(0x42810010, pwmmasks); //Start timer1 with count down/auto reload capability and interrupts.

	/*
	 * We start with the fixed divisor and after every CYCLE_PER_DUTYCYCLE
	 * decrement the divisor by 1, as a result Duty cycle increases
	 * proportionally. This is done until duty cycle is reached upto
	 * MAX_DUTYCYCLE
	 */
//	Div = DUTYCYCLE_DIVISOR;

	/* Configure PWM */
//	do {
//		/* Fail check for 0 divisor */
//		if (!Div) {
//			Status = XST_FAILURE;
//			goto err;
//		}
//	for(Div = 100; Div > 0; Div--){

		/* Disable PWM for reconfiguration */
//		XTmrCtr_PwmDisable(TmrCtrInstancePtr);

		/* Configure PWM */
		Period = PWM_PERIOD;
//		HighTime = (0.025+0.001*Div)*PWM_PERIOD; // (0.025 + .1*ADC/2047)*PWM_PERIOD;
//		HighTime = Period*Div/100;
		HighTime = DUTY_PERIOD; //Period*1/4;

//		DutyCycle = XTmrCtr_PwmConfigure(TmrCtrInstancePtr, Period,
//								HighTime);
//		if (Status != XST_SUCCESS) {
//			Status = XST_FAILURE;
//			goto err;
//		}

//		DutyCycle_percent = (float)HighTime/(float)Period*100;
		Xil_Out32(0x42810000+4, Period/20);
		Xil_Out32(0x42810010+4, HighTime/20);


//		printf("PWM Configured for Duty Cycle = %5.2F\r\n", DutyCycle_percent);

		/* Enable PWM */
//		XTmrCtr_PwmEnable(TmrCtrInstancePtr);
		pwmmasks |= XTC_CSR_ENABLE_PWM_MASK | XTC_CSR_EXT_GENERATE_MASK|XTC_CSR_ENABLE_ALL_MASK;
		Xil_Out32(0x42810000, pwmmasks);
		Xil_Out32(0x42810010, pwmmasks);

//	    for(int Counter = 0; Counter < WAIT_COUNT; Counter++){
//
//			}

//		NoOfCycles = 0;
//		WaitCount = WAIT_COUNT;
//		while (NoOfCycles < CYCLE_PER_DUTYCYCLE) {
//			if (PeriodTimerHit == TRUE && HighTimerHit == TRUE) {
//				PeriodTimerHit = FALSE;
//				HighTimerHit = FALSE;
//				WaitCount = WAIT_COUNT;
//				NoOfCycles++;
//			}
//
//			/* Interrupt did not occur as expected */
//			if (!(--WaitCount)) {
//				return XST_FAILURE;
//			}
//		}
//	} while (DutyCycle < MAX_DUTYCYCLE);
//	}

//	Status = XST_SUCCESS;
//err:
//	/* Disable PWM */
//	XTmrCtr_PwmDisable(TmrCtrInstancePtr);
//
//	/* Disable interrupts */
//	TmrCtrDisableIntr(IntcInstancePtr, DeviceId);

	return Status;
}

/*****************************************************************************/
/**
* This function demonstrates the use of tmrctr PWM APIs.
*
* @param	IntcInstancePtr is a pointer to the Interrupt Controller
*		driver Instance
* @param	TmrCtrInstancePtr is a pointer to the XTmrCtr driver Instance
* @param	DeviceId is the XPAR_<TmrCtr_instance>_DEVICE_ID value from
*		xparameters.h
* @param	IntrId is XPAR_<INTC_instance>_<TmrCtr_instance>_INTERRUPT_INTR
*		value from xparameters.h
*
* @return	XST_SUCCESS if the Test is successful, otherwise XST_FAILURE
*
* @note		none.
*
*****************************************************************************/
int TmrCtrCapture(INTC *IntcInstancePtr, XTmrCtr *TmrCtrInstancePtr,
						u16 DeviceId, u16 IntrId)
{

	int Status;

	/*
	 * Initialize the timer counter so that it's ready to use,
	 * specify the device ID that is generated in xparameters.h
	 */
	Status = XTmrCtr_Initialize(TmrCtrInstancePtr, DeviceId);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Perform a self-test to ensure that the hardware was built
	 * correctly. Timer0 is used for self test
	 */
	Status = XTmrCtr_SelfTest(TmrCtrInstancePtr, TMRCTR_0_0);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Connect the timer counter to the interrupt subsystem such that
	 * interrupts can occur
	 */
	Status = TmrCtrSetupIntrSystem(IntcInstancePtr, TmrCtrInstancePtr,
							DeviceId, IntrId);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Setup the handler for the timer counter that will be called from the
	 * interrupt context when the timer expires
	 */
	XTmrCtr_SetHandler(TmrCtrInstancePtr, TimerCounterHandler_0,
							TmrCtrInstancePtr);

	u32 masks = XTC_CSR_ENABLE_ALL_MASK | XTC_CSR_ENABLE_INT_MASK | XTC_CSR_AUTO_RELOAD_MASK |
			XTC_CSR_EXT_CAPTURE_MASK | XTC_CSR_CAPTURE_MODE_MASK;
	Xil_Out32(0x42800000, masks); //Start timer0 with Capture capability and interrupts.
	Xil_Out32(0x42800010, masks); //Start timer1 with Capture capability and interrupts.

	return Status;
}

/*****************************************************************************/
/**
* This function is the handler which performs processing for the timer counter.
* It is called from an interrupt context.
*
* @param	CallBackRef is a pointer to the callback function
* @param	TmrCtrNumber is the number of the timer to which this
*		handler is associated with.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
//static void TimerCounterHandler_1(void *CallBackRef, u8 TmrCtrNumber)
//{
//	/* Mark if period timer expired */
//	if (TmrCtrNumber == TMRCTR_1_0) {
//		xil_printf("PWM handler - TMRCTR_1_0\r\n");
//		PeriodTimerHit = TRUE;
//	}
//
//	/* Mark if high time timer expired */
//	if (TmrCtrNumber == TMRCTR_1_1) {
//		xil_printf("PWM handler - TMRCTR_1_1\r\n");
//		HighTimerHit = TRUE;
//	}
//}



static void TimerCounterHandler_0(void *CallBackRef, u8 TmrCtrNumber)
{

	XTmrCtr *InstancePtr = (XTmrCtr *)CallBackRef;

	/* Mark if period timer expired */
	if (TmrCtrNumber == TMRCTR_0_0) {
		capture0 = XTmrCtr_GetCaptureValue(InstancePtr, TMRCTR_0_0);
//		capture0 =  Xil_In32(0x42800008);
//		xil_printf("\rIn Tmrctr interrupt handler - TMRCTR_0 %u\r\n", capture0);
		PeriodTimerHit = TRUE;
	}

	/* Mark if high time timer expired */
	if (TmrCtrNumber == TMRCTR_0_1) {
		capture1 = XTmrCtr_GetCaptureValue(InstancePtr, TMRCTR_0_1);
//		capture1 =  Xil_In32(0x42800018);
//		xil_printf("\rIn Tmrctr interrupt handler - TMRCTR_1 %u\r\n", capture1);
		HighTimerHit = TRUE;

		if(capture1 > capture0){
			pulsewidth = capture1 - capture0;
		}
		    else {
		    pulsewidth = capture1 - capture0 + 0xFFFFFFFF;
		}

	}

}

/*****************************************************************************/
/**
* This function setups the interrupt system such that interrupts can occur
* for the timer counter. This function is application specific since the actual
* system may or may not have an interrupt controller.  The timer counter could
* be directly connected to a processor without an interrupt controller.  The
* user should modify this function to fit the application.
*
* @param	IntcInstancePtr is a pointer to the Interrupt Controller
*		driver Instance.
* @param	TmrCtrInstancePtr is a pointer to the XTmrCtr driver Instance.
* @param	DeviceId is the XPAR_<TmrCtr_instance>_DEVICE_ID value from
*		xparameters.h.
* @param	IntrId is XPAR_<INTC_instance>_<TmrCtr_instance>_VEC_ID
*		value from xparameters.h.
*
* @return	XST_SUCCESS if the Test is successful, otherwise XST_FAILURE.
*
* @note		none.
*
******************************************************************************/
static int TmrCtrSetupIntrSystem(INTC *IntcInstancePtr,
			XTmrCtr *TmrCtrInstancePtr, u16 DeviceId, u16 IntrId)
{
	 int Status;

//#else
	XScuGic_Config *IntcConfig;

	/*
	 * Initialize the interrupt controller driver so that it is ready to
	 * use
	 */
	IntcConfig = XScuGic_LookupConfig(INTC_DEVICE_ID);
	if (NULL == IntcConfig) {
		return XST_FAILURE;
	}

	Status = XScuGic_CfgInitialize(IntcInstancePtr, IntcConfig,
					IntcConfig->CpuBaseAddress);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	XScuGic_SetPriorityTriggerType(IntcInstancePtr, IntrId,
					0xA0, 0x3);

	/*
	 * Connect the interrupt handler that will be called when an
	 * interrupt occurs for the device.
	 */
	Status = XScuGic_Connect(IntcInstancePtr, IntrId,
				 (Xil_ExceptionHandler)XTmrCtr_InterruptHandler,
				 TmrCtrInstancePtr);
	if (Status != XST_SUCCESS) {
		return Status;
	}

	/* Enable the interrupt for the Timer device */
	XScuGic_Enable(IntcInstancePtr, IntrId);
//#endif /* XPAR_INTC_0_DEVICE_ID */

	/* Initialize the exception table */
	Xil_ExceptionInit();

	/* Register the interrupt controller handler with the exception table */
	Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_INT,
					(Xil_ExceptionHandler)
					INTC_HANDLER,
					IntcInstancePtr);

	/* Enable non-critical exceptions */
	Xil_ExceptionEnable();

	return XST_SUCCESS;
}

