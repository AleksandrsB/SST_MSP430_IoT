/*
 * Typedefs.h
 *
 *  Created on: 2024. gada 12. marts
 *      Author: Klavins Eriks
 */

#ifndef UTILS_TYPEDEFS_H_
#define UTILS_TYPEDEFS_H_

typedef union{
    unsigned int B;
    struct
    {
       unsigned int Button             :1; // identify that button was pressed

       unsigned int CC_Config          :1;
       unsigned int KalmanReady         :1;

       unsigned int GDO_0_Set          :1;
       unsigned int GDO_2_Set          :1;
    };
}__Status_t;

typedef union{
    unsigned int B;
    struct
    {
       unsigned int ADC_Val         :10;
       unsigned int DataReady       :1; // identify that ADC has finished the conversion
       unsigned int channel         :4;
       unsigned int __reserved      :1;
    };
}__ADC_Status_t;

typedef union{
    float f; // 4 bytes
    unsigned int i[2];
    unsigned char b[4];
}__TypeCast;



#endif /* UTILS_TYPEDEFS_H_ */
