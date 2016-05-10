#ifndef _SAA7118_H_
#define _SAA7118_H_

/************************
 * Analog Input Control *
 ************************/
#define INPUT_CTRL                                      0x02


/* Amplifier and Anti Alias filter 
   Note:
        The Anti Alias can not be set to active when the
        amplifier is not active
 */
#define INPUT_CTRL_FUNCTION_AMPLIFIER_BYPASSED          0x00
#define INPUT_CTRL_FUNCTION_AMPLIFIER_ACTIVE            0x80
#define INPUT_CTRL_FUNCTION_ANTI_ALIAS_BYPASSED         0x00
#define INPUT_CTRL_FUNCTION_ANTI_ALIAS_ACTIVE           0x40

/* Masks */
#define INPUT_CTRL_FUNCTION_MASK                        0xC0
#define INPUT_CTRL_MODE_SELECTION_MASK                  0x3F

/*********************************
 * X-PORT INPUT REFERENCE SIGNAL *
 *********************************/
#define XPORT_INPUT_CONFIG                              0x92

/* Invert Input Field ID */
#define XPORT_INPUT_CONFIG_FIELD_ID_NORMAL              0x00
#define XPORT_INPUT_CONFIG_FIELD_ID_INVERT              0x40

/* Mask */
#define XPORT_INPUT_CONFIG_FIELD_ID_MASK                0x40

/******************************************
 * I-PORT OUTPUT FORMAT AND CONFIGURATION *
 ******************************************/
#define IPORT_CONFIG                                    0x93

/* Enable ITU-656 standard */
#define IPORT_CONFIG_ITU656_ENABLE                      0x80
#define IPORT_CONFIG_ITU656_DISABLE                     0x00

#define IPORT_CONFIG_DATA_SIZE_8_BIT                    0x00
#define IPORT_CONFIG_DATA_SIZE_16_BIT                   0x40

/* Mask */
#define IPORT_CONFIG_ITU656_MASK                        0x80
#define IPORT_CONFIG_DATA_SIZE_MASK                     0x40

/****************************************
 * Horizontal Input Window Start Low Byte *
 ****************************************/
#define HORIZONTAL_INPUT_WINDOW_START_LSB               0x94

/*****************************************
 * Horizontal Input Window Start High Byte *
 *****************************************/
#define HORIZONTAL_INPUT_WINDOW_START_MSB               0x95

/****************************************
 * Vertical Input Window Start Low Byte *
 ****************************************/
#define VERTICAL_INPUT_WINDOW_START_LSB                 0x98

/*****************************************
 * Vertical Input Window Start High Byte *
 *****************************************/
#define VERTICAL_INPUT_WINDOW_START_MSB                 0x99

#endif  /* _SAA7118_H_ */
