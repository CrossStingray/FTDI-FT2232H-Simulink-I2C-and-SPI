// I2C LibMPSSE.cpp :

#include "..\libftd2xx\ftd2xx.h"
#include "..\include\libmpsse_i2c.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define MAG_ADDRESS 0x0C
#define MPU9265_SETUP_ACC_SCALE 28
#define MPU9265_SETUP_GYRO_SCALE 27
#define MPU9265_MAG_REGISTER 0x37
#define MPU9265_MAG_SETUP 0x02
#define MAG_REGISTER 0x0A
#define MAG_SETUP 0x01
#define MAG_BUFF_REGISTER 0x02
#define MAG_READ_REGISTER 0x03
#define GYRO_FULL_SCALE_250_DPS 0x00
#define GYRO_FULL_SCALE_500_DPS 0x08
#define GYRO_FULL_SCALE_1000_DPS 0x10
#define GYRO_FULL_SCALE_2000_DPS 0x18
#define ACC_FULL_SCALE_2_G 0x00
#define ACC_FULL_SCALE_4_G 0x08
#define ACC_FULL_SCALE_8_G 0x10
#define ACC_FULL_SCALE_16_G 0x18

#define ACCEL_XOUT_H       0x3B
#define ACCEL_XOUT_L       0x3C
#define ACCEL_YOUT_H       0x3D
#define ACCEL_YOUT_L       0x3E
#define ACCEL_ZOUT_H       0x3F
#define ACCEL_ZOUT_L       0x40
#define TEMP_OUT_H         0x41
#define TEMP_OUT_L         0x42
#define GYRO_XOUT_H        0x43
#define GYRO_XOUT_L        0x44
#define GYRO_YOUT_H        0x45
#define GYRO_YOUT_L        0x46
#define GYRO_ZOUT_H        0x47
#define GYRO_ZOUT_L        0x48

#define APP_CHECK_STATUS(exp) {if (exp!=FT_OK){printf(" status(0x%x) != FT_OK\n", exp);}else{;}};

/* Handle to MPSSE driver */
void* hdll_mpsse;

FT_STATUS i2c_read(FT_HANDLE ftHandle, UCHAR address, UCHAR reg, PUCHAR value)
{
	FT_STATUS status;
	DWORD xfer = 0;

	status = I2C_DeviceWrite(ftHandle, address, 1, &reg, &xfer,
		I2C_TRANSFER_OPTIONS_START_BIT |
		I2C_TRANSFER_OPTIONS_BREAK_ON_NACK |
		I2C_TRANSFER_OPTIONS_FAST_TRANSFER_BYTES 
	);

	if (status == FT_OK)
	{
		/* Repeated Start condition generated. */
		status = I2C_DeviceRead(ftHandle, address, 1, value, &xfer,
			I2C_TRANSFER_OPTIONS_START_BIT | 
			I2C_TRANSFER_OPTIONS_STOP_BIT |
			I2C_TRANSFER_OPTIONS_NACK_LAST_BYTE |
			I2C_TRANSFER_OPTIONS_FAST_TRANSFER_BYTES
		);
	}
	APP_CHECK_STATUS(status);

	return status;
}


FT_STATUS i2c_write(FT_HANDLE ftHandle, UCHAR address, UCHAR reg, UCHAR value)
{
	FT_STATUS status;
	DWORD xfer = 0;

	status = I2C_DeviceWrite(ftHandle, address, 1, &reg, &xfer,
		I2C_TRANSFER_OPTIONS_START_BIT |
		I2C_TRANSFER_OPTIONS_BREAK_ON_NACK);
	APP_CHECK_STATUS(status);

	if (status == FT_OK)
	{
		/* Register address not sent on register write. */
		status = I2C_DeviceWrite(ftHandle, address, 1, &value, &xfer,
			I2C_TRANSFER_OPTIONS_NO_ADDRESS |
			I2C_TRANSFER_OPTIONS_STOP_BIT |
			I2C_TRANSFER_OPTIONS_BREAK_ON_NACK);
		APP_CHECK_STATUS(status);
	}

	return status;
}

int main(void)
{
	DWORD channel;
	DWORD channels;
	FT_HANDLE ftHandle;
	ChannelConfig channelConf;
	FT_STATUS status;
	UCHAR address = 0x68; //I2C Address for the MPU9265 sensor

	FT_DEVICE_LIST_INFO_NODE devList;

	channelConf.ClockRate = I2C_CLOCK_HIGH_SPEED_MODE;
	channelConf.LatencyTimer = 1;
	channelConf.Options = 0;

	Init_libMPSSE();

	printf("\nTest case 1 - I2C_GetNumChannels\n");
	status = I2C_GetNumChannels(&channels);
	printf("		I2C_GetNumChannels returned %d; channels=%d\n", status, channels);

	printf("\nTest case 2 - I2C_GetChannelInfo\n");
	for (channel = 0; channel < channels; channel++)
	{
		status = I2C_GetChannelInfo(channel, &devList);
		printf("		I2C_GetNumChannels returned %d for channel =%d\n", status, channel);
		/*print the dev info*/
		printf("		Flags=0x%x\n", devList.Flags);
		printf("		Type=0x%x\n", devList.Type);
		printf("		ID=0x%x\n", devList.ID);
		printf("		LocId=0x%x\n", devList.LocId);
		printf("		SerialNumber=%s\n", devList.SerialNumber);
		printf("		Description=%s\n", devList.Description);
		printf("		ftHandle=%p (should be zero)\n", devList.ftHandle);
	}

	if (channels > 1)
	{
		channel = 1;

		// test: read ID register
		status = I2C_OpenChannel(channel, &ftHandle);
		APP_CHECK_STATUS(status);
		printf("Channel %d open status=%d\n", channel, status);

		status = I2C_InitChannel(ftHandle, &channelConf);

		Sleep(2);

		// Write config Register for accelerometer
		status = i2c_write(ftHandle, address, MPU9265_SETUP_ACC_SCALE, ACC_FULL_SCALE_16_G);
		APP_CHECK_STATUS(status);

		// Write config register for gyroscope
		status = i2c_write(ftHandle, address, MPU9265_SETUP_GYRO_SCALE, GYRO_FULL_SCALE_2000_DPS);
		APP_CHECK_STATUS(status);


		for (int i = 0; i < 1000; i++) {
			// Read accelerometer and gyroscope
			UCHAR Buf[14];
			//status = i2c_read_multi(ftHandle, address, ACCEL_XOUT_H, Buf, (UCHAR)14);
			//APP_CHECK_STATUS(status);
			/*
			status = i2c_read(ftHandle, address, ACCEL_XOUT_H, &Buf[0]);
			status = i2c_read(ftHandle, address, ACCEL_XOUT_L, &Buf[1]);
			status = i2c_read(ftHandle, address, ACCEL_YOUT_H, &Buf[2]);
			status = i2c_read(ftHandle, address, ACCEL_YOUT_L, &Buf[3]);
			status = i2c_read(ftHandle, address, ACCEL_ZOUT_H, &Buf[4]);
			status = i2c_read(ftHandle, address, ACCEL_ZOUT_L, &Buf[5]);
			status = i2c_read(ftHandle, address, TEMP_OUT_H, &Buf[6]);
			status = i2c_read(ftHandle, address, TEMP_OUT_L, &Buf[7]);
			status = i2c_read(ftHandle, address, GYRO_XOUT_H, &Buf[8]);
			status = i2c_read(ftHandle, address, GYRO_XOUT_H, &Buf[10]);
			status = i2c_read(ftHandle, address, GYRO_YOUT_H, &Buf[9]);
			status = i2c_read(ftHandle, address, GYRO_YOUT_L, &Buf[11]);
			status = i2c_read(ftHandle, address, GYRO_ZOUT_H, &Buf[12]);
			status = i2c_read(ftHandle, address, GYRO_ZOUT_L, &Buf[13]);
			APP_CHECK_STATUS(status);
			*/

			
			for (int i = 0; i < 14; i++) {
				status = i2c_read(ftHandle, address, ACCEL_XOUT_H + i, &Buf[i]);
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

			// Sleep in miliseconds
			// Sleep(50);
		}
		status = I2C_CloseChannel(ftHandle);
		APP_CHECK_STATUS(status);
	}
	Cleanup_libMPSSE();

	return 0;
}
