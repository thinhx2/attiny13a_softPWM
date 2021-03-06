///////////////////////////////////////////////////////////////// 30 + 30 минут	//////////////////////////////////////////////////////////////////////////
#include <avr/io.h>			//   зависит от ширины фазы	//   Attiny13a, Fuses =0xFF6A, CLK=9.6 MHz, Flash=94 (+18-26) b, SRAM=0b, EEPROM=0b	//
#include <avr/interrupt.h>	//    переменные в регистрах	//												 Автор: Dolphin, 18.01.2015 13:00	//
#include <util/delay.h>		//    память не используется	////////////////////////////////////////////////////////////////////////////////////////////
register  uint8_t 	mask asm("r14"), phase asm("r16");	// mask - конфигурационная маска импульсов, phase - фаза сигнала						//
ISR (TIM0_COMPA_vect)									// - обработчик прерывания по совпадению таймера TIM0 с регистром OCR0A				//
{// 	Режимы обработки относительных смещений импульсов      //(нужный режим раскомментировать, или реализовать обработчик переключения режимов)//
 // режим  №1 относительное смещение на импульс :			//		F (max) ~ 75.500 kHz, скважность ШИМ = 33.3%				  (+20 байт кода)	//
 	if (++phase<3) mask+=phase; else {mask=1; phase=0;}		//			цикл фаз (PB2 | PB1 | PB0): 001, 010, 100 ...			          (период 3 фазы)	//
 // режим  №2	..........	на треть импульса :				//---------------------------------------------------------------------------------------------------------------------------------------------//
 /*asm volatile   ("	cpi		R16,0x06		""\n\t"		//		F (max) ~ 37.400 kHz, скважность ШИМ = 50%	 			  (+26 байт кода)	//
				"	breq	_newPulse		""\n\t"		//			цикл фаз: 001, 011, 010, 110, 100, 101 ...				    (период 6 фаз)	//
				"	cpi		R16,0x01		""\n\t"		//																				//
				"	breq	_nextPhase		""\n\t"		//		ассемблерный код идентичен следующему, на языке Си:							//
				"	cpi		R16,0x03		""\n\t"		//		if (phase != 6)									 							//
				"	breq	_nextPhase		""\n\t"		//			mask = (phase==1 || phase== 3 ) ? (mask+phase+1) : phase;					//
				"	mov		R14,R16			""\n\t"		//		else 																		//
				"	rjmp	_maskReady		""\n\t"		//		{																		//
"_newPulse:"	"	clr		R14				""\n\t"		//			phase=0;							первоначальный вариант				//
				"	clr		R16				""\n\t"		//			mask=1;								      которого занял 58 байт,		//
"_nextPhase:"	"	add		R14,R16			""\n\t"		//		}												при F (max) ~ 28.500 kHz	//
				"	inc		R14				""\n\t"		//		phase++;																	//
"_maskReady:"	"	inc		R16				""\n\t");	*///---------------------------------------------------------------------------------------------------------------------------------------------//
 // режим  №3	..........	на половину импульса :				//		F (max) ~ 75.500 kHz, скважность ШИМ = 66.6%				  (+18 байт кода)	//
// 	if (++phase<3) mask-=phase; else {mask+=phase; phase=0;}	//			цикл фаз: 101, 011, 110 ...(период 3 фазы)	//
//		TCNT0 = 238 для режима №1 (2,48mS=400Hz)
//
 		PORTB = mask; TCNT0 = 238;						// - вывод сконфигурированных значений фаз маской и сброс таймера (длительность фазы)	//
}//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//
int main(void) {							// инициализирую переменные, порт ввода/вывода и прерывание по таймеру							//
	register uint8_t width asm("r4")= 0x0C;	//  - ширина фазы (если ниже порога пропуска, то выполняется полный цикл таймера на импульс,		//
	mask = 0x06; phase = 0x00; DDRB = 0x07;	//  - PB0, PB1, PB2 выходы 											               т.е. нижний предел F)	//
	TIMSK0 = (0 << OCIE0B) | (1 << OCIE0A);	//  - маска прерывания на совпадение таймера с регистром сравнения OCR0A							//
	TCCR0B |= (1 << CS02)|(0 << CS01)|(0 << CS00);  TCNT0=0;  sei();	//  - обнуляю таймер и запускаю на частоте CLK без предделителя									//
	while (!0) {							//  - основной цикл освобождён для произвольных вычислений											//
		OCR0A = width;						//  - значение ширины фазы в регистр сравнения OCR0A, при достижении счетчиком - прерывание		//
//		++width;_delay_ms(20); // для тестов	//  ИТОГ: 3 режима, скважность 33.3%, 50%, 66.6%, F - обратно-пропорциональна ширине фазы			//
	}										//  При использовании внешнего генератора, верхний предел F ~ 160 KHz			(c) by Dolphin (2015)	//
}//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
