#ifndef _QMI8658REG_H_
#define _QMI8658REG_H_

#define QMI8658_SLAVE_ADDR_L			0x6a
#define QMI8658_SLAVE_ADDR_H			0x6b

#define QMI8658_DISABLE_ALL				(0x0)
#define QMI8658_ACC_ENABLE				(0x1)
#define QMI8658_GYR_ENABLE				(0x2)
#define QMI8658_ACCGYR_ENABLE			(QMI8658_ACC_ENABLE | QMI8658_GYR_ENABLE)

#define QMI8658_INT1_ENABLE				0x08
#define QMI8658_INT2_ENABLE				0x10

enum Qmi8658Register {
	Qmi8658Register_WhoAmI = 0,
	Qmi8658Register_Revision,
	Qmi8658Register_Ctrl1,
	Qmi8658Register_Ctrl2,
	Qmi8658Register_Ctrl3,
	Qmi8658Register_Ctrl4,
	Qmi8658Register_Ctrl5,
	Qmi8658Register_Ctrl6,
	Qmi8658Register_Ctrl7,
	Qmi8658Register_Ctrl8,
	Qmi8658Register_Ctrl9,
	Qmi8658Register_Status0 = 46,
	Qmi8658Register_Status1,
	Qmi8658Register_Ax_L = 53,
	Qmi8658Register_Ax_H,
	Qmi8658Register_Ay_L,
	Qmi8658Register_Ay_H,
	Qmi8658Register_Az_L,
	Qmi8658Register_Az_H,
	Qmi8658Register_Gx_L = 59,
	Qmi8658Register_Gx_H,
	Qmi8658Register_Gy_L,
	Qmi8658Register_Gy_H,
	Qmi8658Register_Gz_L,
	Qmi8658Register_Gz_H,
	Qmi8658Register_Reset = 96
};

enum qmi8658_Ctrl9Command {
	qmi8658_Ctrl9_Cmd_NOP					= 0X00,
	qmi8658_Ctrl9_Cmd_On_Demand_Cali		= 0xA2
};

enum qmi8658_LpfConfig { Qmi8658Lpf_Disable, Qmi8658Lpf_Enable };
enum qmi8658_StConfig { Qmi8658St_Disable, Qmi8658St_Enable };

enum qmi8658_AccRange {
	Qmi8658AccRange_2g = 0x00 << 4,
	Qmi8658AccRange_4g = 0x01 << 4,
	Qmi8658AccRange_8g = 0x02 << 4,
	Qmi8658AccRange_16g = 0x03 << 4
};

enum qmi8658_AccOdr {
	Qmi8658AccOdr_1000Hz = 0x03,
	Qmi8658AccOdr_500Hz = 0x04,
	Qmi8658AccOdr_250Hz = 0x05,
	Qmi8658AccOdr_125Hz = 0x06
};

enum qmi8658_GyrRange {	
	Qmi8658GyrRange_2048dps = 7 << 4
};

enum qmi8658_GyrOdr {
	Qmi8658GyrOdr_1000Hz = 0x03,
	Qmi8658GyrOdr_500Hz	= 0x04,
	Qmi8658GyrOdr_250Hz	= 0x05,
	Qmi8658GyrOdr_125Hz	= 0x06
};

typedef struct {
	unsigned char enSensors;
	enum qmi8658_AccRange accRange;
	enum qmi8658_AccOdr accOdr;
	enum qmi8658_GyrRange gyrRange;
	enum qmi8658_GyrOdr gyrOdr;
} qmi8658_config;

typedef struct {
	qmi8658_config cfg;
	unsigned short ssvt_a;
	unsigned short ssvt_g;
	float imu[6];
} qmi8658_state;

#endif
