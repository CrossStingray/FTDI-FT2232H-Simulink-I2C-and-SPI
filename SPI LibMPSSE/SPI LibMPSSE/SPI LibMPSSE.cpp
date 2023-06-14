// SPI LibMPSSE.cpp : Este archivo contiene la función "main". La ejecución del programa comienza y termina ahí.
//

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <windows.h>

#include "..\libftd2xx\ftd2xx.h"
#include "..\include\libmpsse_spi.h"

#define MPU9265_SETUP_ACC_SCALE 28
#define MPU9265_SETUP_GYRO_SCALE 27
#define GYRO_FULL_SCALE_250_DPS 0x00
#define GYRO_FULL_SCALE_500_DPS 0x08
#define GYRO_FULL_SCALE_1000_DPS 0x10
#define GYRO_FULL_SCALE_2000_DPS 0x18
#define ACC_FULL_SCALE_2_G 0x00
#define ACC_FULL_SCALE_4_G 0x08
#define ACC_FULL_SCALE_8_G 0x10
#define ACC_FULL_SCALE_16_G 0x18

#define WHO_AM_I 0x75
#define WHO_AM_I_9250_ANS 0x71
#define GYRO_CONFIG 0x1B
#define ACCEL_CONFIG 0x1C


#define APP_CHECK_STATUS(exp) {if (exp!=FT_OK){printf(" status(0x%x) != FT_OK\n", exp);}else{;}};

FT_STATUS spi_read(FT_HANDLE ftHandle, UCHAR reg, PUCHAR value)
{
	FT_STATUS status;
	DWORD xfer = 0;
	reg = (reg | (1 << 7)); // Sets a 1 on the MSB of the reg byte, it indicates a read operation on the MPU9250

	status = SPI_Write(ftHandle, &reg, 1, &xfer,
		SPI_TRANSFER_OPTIONS_SIZE_IN_BYTES |
		SPI_TRANSFER_OPTIONS_CHIPSELECT_ENABLE
	);

	if (status == FT_OK)
	{
		status = SPI_Read(ftHandle, value, 1, &xfer,
			SPI_TRANSFER_OPTIONS_SIZE_IN_BYTES |
			SPI_TRANSFER_OPTIONS_CHIPSELECT_DISABLE
		);
	}
	APP_CHECK_STATUS(status);

	return status;
}

FT_STATUS spi_write(FT_HANDLE ftHandle, UCHAR reg, UCHAR value)
{
	FT_STATUS status;
	DWORD xfer = 0;

	status = SPI_Write(ftHandle, &reg, 1, &xfer,
		SPI_TRANSFER_OPTIONS_SIZE_IN_BYTES |
		SPI_TRANSFER_OPTIONS_CHIPSELECT_ENABLE
	);
	APP_CHECK_STATUS(status);

	if (status == FT_OK)
	{
		/* Register address not sent on register write. */
		status = SPI_Write(ftHandle, &value, 1, &xfer,
			SPI_TRANSFER_OPTIONS_SIZE_IN_BYTES |
			SPI_TRANSFER_OPTIONS_CHIPSELECT_DISABLE
		);
		APP_CHECK_STATUS(status);
	}

	return status;
}

int main()
{
	FT_HANDLE ftHandle;
	FT_DEVICE_LIST_INFO_NODE devList;
	ChannelConfig channelConf;
	DWORD channel;
	DWORD channels;
	FT_STATUS status;

	channelConf.ClockRate = 1000000;
	channelConf.LatencyTimer = 1;
	channelConf.configOptions = SPI_CONFIG_OPTION_MODE0 | SPI_CONFIG_OPTION_CS_DBUS3 | SPI_CONFIG_OPTION_CS_ACTIVELOW;

	Init_libMPSSE();

	status = SPI_GetNumChannels(&channels);
	for (channel = 0; channel < channels; channel++)
	{
		status = SPI_GetChannelInfo(channel, &devList);
		printf("SPI_GetNumChannels returned %d for channel %d\n", status, channel);
		/*print the dev info*/
		printf("		VID/PID: 0x%04x/0x%04x\n", devList.ID >> 16, devList.ID & 0xffff);
		printf("		SerialNumber: %s\n", devList.SerialNumber);
		printf("		Description: %s\n", devList.Description);
	}

	if (channels > 1)
	{
		// Open the the channel specified by the USE_MPSSE macro.
		// This must be defined to get this far.
		channel = 1;

		status = SPI_OpenChannel(channel, &ftHandle);
		if (status != FT_OK)
		{
			fprintf(stderr, "Channel %d failed to open status %d\n", channel, status);
			exit(-2);
		}
		status = SPI_InitChannel(ftHandle, &channelConf);
		if (status != FT_OK)
		{
			fprintf(stderr, "Channel %d failed to initialise SPI status %d\n", channel, status);
			exit(-3);
		}

		Sleep(10);
		//Begins SPI data transfers

		status = spi_write(ftHandle, 0x6B, 0x80); //Resets device
		APP_CHECK_STATUS(status);

		status = spi_write(ftHandle, 0x6A, 0x10);
		APP_CHECK_STATUS(status);

		UCHAR temp;
		status = spi_read(ftHandle, WHO_AM_I, &temp);
		APP_CHECK_STATUS(status);
		printf("Who am I 0x%x\n", temp);

		// Write config Register for accelerometer
		status = spi_write(ftHandle, MPU9265_SETUP_ACC_SCALE, ACC_FULL_SCALE_16_G);
		APP_CHECK_STATUS(status);

		// Write config register for gyroscope
		status = spi_write(ftHandle, MPU9265_SETUP_GYRO_SCALE, GYRO_FULL_SCALE_2000_DPS);
		APP_CHECK_STATUS(status);
		

		UCHAR Buf[14];

		printf("Press ESC key to exit!\n");
		Sleep(1000);

		bool is_ESC_Key_Pressed = false;
		while( !is_ESC_Key_Pressed) {

			if (GetAsyncKeyState(VK_ESCAPE)) {
				is_ESC_Key_Pressed = true;
			}

			for (int i = 0; i < 14; i++) {
				status = spi_read(ftHandle, (0x3B + i), &Buf[i]);
			}
			APP_CHECK_STATUS(status);

			// Convert accelerometer registers
			int16_t ax = Buf[0] << 8 | Buf[1];
			int16_t ay = Buf[2] << 8 | Buf[3];
			int16_t az = Buf[4] << 8 | Buf[5];

			int16_t temp = Buf[6] << 8 | Buf[7];
			// Convert gyroscope registers
			int16_t gx = Buf[8] << 8 | Buf[9];
			int16_t gy = Buf[10] << 8 | Buf[11];
			int16_t gz = Buf[12] << 8 | Buf[13];

			printf("ax = %-6d, ay = %-6d, az = %-6d\tgx = %-6d, gy = %-6d, gz = %-6d\t t = %-6d\n",
				ax, ay, az, gx, gy, gz, temp);
		}

		//printf("Read: %s\n", Buf);

		status = SPI_CloseChannel(ftHandle);

		APP_CHECK_STATUS(status);

	}
	else
	{
		fprintf(stderr, "No SPI channels found\n");
		exit(-1);
	}

	Cleanup_libMPSSE();

}

