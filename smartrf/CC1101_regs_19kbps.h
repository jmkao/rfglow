/* Address Config = No address check */
/* Base Frequency = 915.999756 */
/* CRC Autoflush = true */
/* CRC Enable = true */
/* Carrier Frequency = 915.999756 */
/* Channel Number = 0 */
/* Channel Spacing = 249.938965 */
/* Data Format = Normal mode */
/* Data Rate = 19.1917 */
/* Deviation = 20.629883 */
/* Device Address = 5 */
/* Manchester Enable = false */
/* Modulated = true */
/* Modulation Format = GFSK */
/* PA Ramping = false */
/* Packet Length = 6 */
/* Packet Length Mode = Fixed packet length mode. Length configured in PKTLEN register */
/* Preamble Count = 4 */
/* RX Filter BW = 101.562500 */
/* Sync Word Qualifier Mode = 30/32 sync word bits detected */
/* TX Power = 0 */
/* Whitening = true */

// RF settings for Panstamp
// Exported from SmartRF Sutdio

#define CC1101_DEFVAL_IOCFG2     0x2E // GDO2 Output Pin Configuration
#define CC1101_DEFVAL_IOCFG1     0x2E // GDO1 Output Pin Configuration
#define CC1101_DEFVAL_IOCFG0     0x06 // GDO0 Output Pin Configuration
#define CC1101_DEFVAL_FIFOTHR    0x47 // RX FIFO and TX FIFO Thresholds
#define CC1101_DEFVAL_SYNC1      0x2C // Sync Word, High Byte
#define CC1101_DEFVAL_SYNC0      0x6E // Sync Word, Low Byte
#define CC1101_DEFVAL_PKTLEN     0x06 // Packet Length
#define CC1101_DEFVAL_PKTCTRL1   0x0C // Packet Automation Control
#define CC1101_DEFVAL_PKTCTRL0   0x44 // Packet Automation Control
#define CC1101_DEFVAL_ADDR       0x05 // Device Address
#define CC1101_DEFVAL_CHANNR     0x00 // Channel Number
#define CC1101_DEFVAL_FSCTRL1    0x06 // Frequency Synthesizer Control
#define CC1101_DEFVAL_FSCTRL0    0x00 // Frequency Synthesizer Control
#define CC1101_DEFVAL_FREQ2      0x23 // Frequency Control Word, High Byte
#define CC1101_DEFVAL_FREQ1      0x3B // Frequency Control Word, Middle Byte
#define CC1101_DEFVAL_FREQ0      0x13 // Frequency Control Word, Low Byte
#define CC1101_DEFVAL_MDMCFG4    0xC9 // Modem Configuration
#define CC1101_DEFVAL_MDMCFG3    0x83 // Modem Configuration
#define CC1101_DEFVAL_MDMCFG2    0x13 // Modem Configuration
#define CC1101_DEFVAL_MDMCFG1    0x23 // Modem Configuration
#define CC1101_DEFVAL_MDMCFG0    0x3B // Modem Configuration
#define CC1101_DEFVAL_DEVIATN    0x35 // Modem Deviation Setting
#define CC1101_DEFVAL_MCSM2      0x07 // Main Radio Control State Machine Configuration
#define CC1101_DEFVAL_MCSM1      0x30 // Main Radio Control State Machine Configuration
#define CC1101_DEFVAL_MCSM0      0x18 // Main Radio Control State Machine Configuration
#define CC1101_DEFVAL_FOCCFG     0x16 // Frequency Offset Compensation Configuration
#define CC1101_DEFVAL_BSCFG      0x6C // Bit Synchronization Configuration
#define CC1101_DEFVAL_AGCCTRL2   0x43 // AGC Control
#define CC1101_DEFVAL_AGCCTRL1   0x40 // AGC Control
#define CC1101_DEFVAL_AGCCTRL0   0x91 // AGC Control
#define CC1101_DEFVAL_WOREVT1    0x87 // High Byte Event0 Timeout
#define CC1101_DEFVAL_WOREVT0    0x6B // Low Byte Event0 Timeout
#define CC1101_DEFVAL_WORCTRL    0xF8 // Wake On Radio Control
#define CC1101_DEFVAL_FREND1     0x56 // Front End RX Configuration
#define CC1101_DEFVAL_FREND0     0x10 // Front End TX Configuration
#define CC1101_DEFVAL_FSCAL3     0xE9 // Frequency Synthesizer Calibration
#define CC1101_DEFVAL_FSCAL2     0x2A // Frequency Synthesizer Calibration
#define CC1101_DEFVAL_FSCAL1     0x00 // Frequency Synthesizer Calibration
#define CC1101_DEFVAL_FSCAL0     0x1F // Frequency Synthesizer Calibration
#define CC1101_DEFVAL_RCCTRL1    0x41 // RC Oscillator Configuration
#define CC1101_DEFVAL_RCCTRL0    0x00 // RC Oscillator Configuration
#define CC1101_DEFVAL_FSTEST     0x59 // Frequency Synthesizer Calibration Control
#define CC1101_DEFVAL_PTEST      0x7F // Production Test
#define CC1101_DEFVAL_AGCTEST    0x3F // AGC Test
#define CC1101_DEFVAL_TEST2      0x81 // Various Test Settings
#define CC1101_DEFVAL_TEST1      0x35 // Various Test Settings
#define CC1101_DEFVAL_TEST0      0x09 // Various Test Settings
