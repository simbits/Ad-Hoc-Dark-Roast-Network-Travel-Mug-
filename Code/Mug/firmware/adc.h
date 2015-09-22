#ifndef __ADC_H__
#define __ADC_H__

#define ADC_PRESCALER_2		0
#define ADC_PRESCALER_4		2
#define ADC_PRESCALER_8		3
#define ADC_PRESCALER_16	4
#define ADC_PRESCALER_32	5
#define ADC_PRESCALER_64	6
#define ADC_PRESCALER_128	7

#define ADC_REF_AREF		0
#define ADC_REF_AVCC		1
#define ADC_REF_1_1V		2
#define ADC_REF_2_56V		3

#define ADC_RIGHT_ADJUST	0
#define ADC_LEFT_ADJUST		1

#define ADC_MUX_MASK		0x1f

/* initialize ADC and perform first conversion */
#define adcInitialize(__p, __r, __a)					\
	do {								\
		while (ADCSRA & _BV(ADSC))				\
			;						\
		ADMUX = (((__r) << REFS0) | ((__a) << ADLAR));		\
		ADCSRA = (0xc0 | (__p));				\
	} while (0)

#define adcStartConversionLeftAdjust(__m)				\
	do {								\
		uint8_t muxreg = ADMUX & (~ADC_MUX_MASK);		\
		while (ADCSRA & _BV(ADSC))				\
			;						\
		ADMUX = (muxreg | _BV(ADLAR) | (__m));			\
	} while (0)


#define adcStartConversion(__m)						\
	do {								\
		uint8_t muxreg = ADMUX & (~ADC_MUX_MASK);		\
		while (ADCSRA & _BV(ADSC))				\
			;						\
		ADMUX = (muxreg | (__m));				\
		ADCSRA |= _BV(ADSC);					\
	} while (0)

inline uint16_t adcConversionResultLeftAdjust(void)	
{
	while (ADCSRA & _BV(ADSC))
		;
	return ADCH;
}
		

inline uint16_t adcConversionResult(void)
{
	uint16_t result;

	while (ADCSRA & _BV(ADSC))
		;
	
	result = ADCL;
	result |= (ADCH << 8);

	return result;
}

#endif /* __ADC_H__ */
