/*
  * Copyright (c) 24-25, Swedish Institute of Computer Science.
  * Все права зарезервированы. *
  * Повторное распространение, использование в исходном и двоичном виде,
  * с модификацией или без - разрешается, если выполняются следующие
  * условия:
  * 1. Распространение исходного кода должно сохранить вышеуказанную пометку
  *    копирайта, этот список условий и следующую правовую оговорку.
  * 2. Распространение исходного кода должно сохранить вышеуказанную пометку
  *    копирайта, этот список условий и следующую правовую оговорку в
  *    документации и/или других материалах, которые будут предоставлены
  *    вместе с распространяемыми материалами.
  * 3. Имя автора не может использоваться, чтобы подтвердить или продвинуть
  *    продукты, написанные с использованием этого программного обеспечения
  *    без специального на то разрешения.
  *
  * ЭТО ПРОГРАММНОЕ ОБЕСПЕЧЕНИЕ ПРЕДОСТАВЛЯЕТСЯ АВТОРОМ ``КАК ЕСТЬ'', БЕЗ
  * КАКОЙ-ЛИБО ЛЮБОЙ РАСШИРЕННОЙ ИЛИ ПОДРАЗУМЕВАЕМОЙ ГАРАНТИИ, ВКЛЮЧАЯ,
  * НО НЕ ОГРАНИЧИВАЯСЬ ЭТИМ, ГАРАНТИИ ВЫСОКОГО СПРОСА И ПРИГОДНОСТИ
  * ДЛЯ КОНКРЕТНОЙ ЦЕЛИ. АВТОР НИ ПРИ КАКИХ УСЛОВИЯХ НЕ ОТВЕТСТВЕНЕН
  * ЗА ЛЮБЫЕ УБЫТКИ - ПРЯМЫЕ, КОСВЕННЫЕ, СЛУЧАЙНЫЕ, СПЕЦИАЛЬНЫЕ, ОБРАЗЦОВЫЕ
  * ИЛИ ПОСЛЕДОВАТЕЛЬНЫЕ (ВКЛЮЧАЯ, НО НЕ ОГРАНИЧИВАЯСЬ ЭТИМ, ТРЕБОВАНИЯ
  * ЗАМЕНЫ ТОВАРА ИЛИ СЕРВИСА; ПОТЕРИ ИСПОЛЬЗОВАНИЯ, ДАННЫХ ИЛИ ВЫГОДЫ;
  * ИЛИ ПРЕКРАЩЕНИЕ БИЗНЕСА), ОДНАКО ВЫЗВАННЫЕ ПО ЛЮБОЙ ТЕОРИИ ОТВЕТСТВЕННОСТИ,
  * ЛИБО В КОНТРАКТЕ, ПРЯМОЙ ОТВЕТСТВЕННОСТИ, ЛИБО В НАРУШЕНИИ ЗАКОННЫХ ПРАВ
  * (ВКЛЮЧАЯ ТАК ИЛИ ИНАЧЕ НЕБРЕЖНОСТЬ), ВОЗНИКАЮЩИЕ ВСЕГДА ИЗ ИСПОЛЬЗОВАНИЯ
  * ЭТОГО ПРОГРАММНОГО ОБЕСПЕЧЕНИЯ, ДАЖЕ ЕСЛИ БЫЛО ПРЕДУПРЕЖДЕНИЕ О ВОЗМОЖНОСТИ
  * ТАКОГО ПОВРЕЖДЕНИЯ.
  *
  * Этот файл является частью стека uIP TCP/IP.
  *
  * Author: Adam Dunkels <adam@sics.se>
  *
  * $Id: pt.h,v 1.2 26/06/12 08::30 adam Exp $
  */
 
 /**
  * \addtogroup pt
  * @{
  */
 
 /**
  * \file
  * Реализация protothread-ов.
  * \author
  * Adam Dunkels <adam@sics.se>
  *
  */
 
 #ifndef __PT_H__
 #define __PT_H__
 
 #include "lc.h"
 
 typedef struct {
   lc_t lc;
 }pt_t;
 
 #define PT_WAITING 0
 #define PT_EXITED  1
 #define PT_ENDED   2
 #define PT_YIELDED 3
 
 /**
  * \name Initialization
  * @{
  */
 
 /**
  * Инициализация protothread.
  *
  * Инициализирует protothread. Инициализация должна быть выполнена до начала
  * выполнения protothread.
  *
  * \param pt Указатель на protothread control structure (структура управления).
  *
  * \sa PT_SPAWN()
  *
  * \hideinitializer
  */
 #define PT_INIT(pt)   LC_INIT((pt)->lc)
 
 /** @} */
 
 /**
  * \name Декларация и определение
 * @{
  */
 
 /**
  * Декларация protothread.
  *
  * Этот макрос используется для декларации protothread. Все protothread-ы должны
  * быть задекларированы с помощью этого макроса.
  *
  * \param name_args Имя и аргументы функции C, реализующей protothread.
  *
  * \hideinitializer
  */
#define PT_THREAD(name_args) char name_args
 
 /**
  * Декларирует начало protothread внутри функции C, реализующей protothread.
  *
  * Этот макрос используется для декларирования стартовой точки для
  * protothread. Она должна быть размещена в начале функции, в которой
  * запускается protothread. Все операторы C до вызова PT_BEGIN()
  * будут выполнены каждый раз, когда назначается шедулинг для protothread.
  *
  * \param pt Указатель на protothread control structure.
  *
  * \hideinitializer
  */
 #define PT_BEGIN(pt) { volatile char PT_YIELD_FLAG = 1; LC_RESUME((pt)->lc)
 
 /**
  * Декларируется конец для protothread.
  *
  * Этот макрос используется для декларирования места, где заканчивается 
  * protothread. Он должен всегда использоваться совместно с соответствующим 
  * парным вызовом макроса PT_BEGIN().
  *
  * \param pt Указатель на protothread control structure.
  *
  * \hideinitializer
  */
 #define PT_END(pt) LC_END((pt)->lc); PT_YIELD_FLAG = 0; \
                    PT_INIT(pt); return PT_ENDED; }
 
 /** @} */
 
 /**
  * \name Блокирующее ожидание.
  * @{
  */
 
 /**
  * Блокирует выполнения и ждет, пока условие не станет true.
  *
  * Этот макрос блокирует выполнение protothread, пока указанное условие не
  * станет равным true.
  *
  * \param pt Указатель на protothread control structure.
  * \param condition Условие.
  *
  * \hideinitializer
  */
 #define PT_WAIT_UNTIL(pt, condition)            \
   do {                                          \
     LC_SET((pt)->lc);                           \
     if(!(condition)) {                          \
       return PT_WAITING;                        \
     }                                           \
   } while(0)
 
 /**
  * Блокировка и ожидание, пока условие true.
  *
  * Эта функция блокирует выполнение и ждет, пока условие в состоянии true.
  * См. также PT_WAIT_UNTIL().
  *
  * \param pt Указатель на protothread control structure.
  * \param cond Условие.
  *
  * \hideinitializer
  */
 #define PT_WAIT_WHILE(pt, cond)  PT_WAIT_UNTIL((pt), !(cond))
 
 /** @} */
 
 /**
  * \name Иераржия protothread-ов
  * @{
  */
 
 /**
  * Блокирует и ждет, пока не завершится дочерний protothread.
  *
  * Этот макрос запускает шедулинг дочернего protothread. Текущий protothread
  * будет заблокирован, пока дочерний protothread не завершится.
  *
  * \note Дочерний protothread должен быть инициализирован вручную функцией
  * PT_INIT() до того, как будет использована функция PT_SPAWN.
  *
  * \param pt Указатель на protothread control structure.
  * \param thread Дочерний protothread с аргументами.
  *
  * \sa PT_SPAWN()
  *
  * \hideinitializer
  */
 #define PT_WAIT_THREAD(pt, thread) PT_WAIT_WHILE((pt), PT_SCHEDULE(thread))
 
 /**
  * Порождает дочерний protothread и ждет выхода из него.
  *
  * Этот макрос порождает дочерний protothread и ждет, пока он не выполнит
  * выход. Макрос может быть использован только в пределах protothread.
  * macro can only be used within a protothread.
  *
  * \param pt Указатель на protothread control structure.
  * \param child Указатель на protothread control structure дочернего protothread.
  * \param thread Дочерний protothread с аргументами.
  *
  * \hideinitializer
  */
 #define PT_SPAWN(pt, child, thread)             \
   do {                                          \
     PT_INIT((child));                           \
     PT_WAIT_THREAD((pt), (thread));             \
   } while(0)
 
 /** @} */
 
 /**
  * \name Выход и рестарт
  * @{
  */
 
 /**
  * Делает рестарт для protothread.
  *
  * Этот макрос заблокирует и перезапустит работающий protothread
  * с того места, где стоит вызов PT_BEGIN().
  *
  * \param pt Указатель на protothread control structure.
  *
  * \hideinitializer
  */
 #define PT_RESTART(pt)                          \
   do {                                          \
     PT_INIT(pt);                                \
     return PT_WAITING;                  \
   } while(0)
 
 /**
  * Выход из protothread.
  *
  * Этот макрос производит выход из protothread. Если protothread был
  * порожден другим protothread, то родительский protothread станет
  * разблокированным и сможет продолжить работу.
  *
  * \param pt Указатель на protothread control structure.
  *
  * \hideinitializer
  */
 #define PT_EXIT(pt)                             \
   do {                                          \
     PT_INIT(pt);                                \
     return PT_EXITED;                   \
   } while(0)
 
 /** @} */
 
 /**
  * \name Вызов protothread
  * @{
  */
 
 /**
  * Шедулинг для protothread.
  *
  * Эта функция осуществляет шедулинг для protothread. Возвращаемое значение
  * функции не равно 0, если protothread работает, или 0, если protothread 
  * сделал выход.
  *
  * \param f Вызов C-функции, которая реализует шедулинг для protothread.
  *
  * \hideinitializer
  */
 #define PT_SCHEDULE(f) ((f) == PT_WAITING)
 
 /** @} */
 
 /**
  * \name Уступка контекста (Yielding) из protothread
  * @{
  */
 
 /**
  * Уступка контекста управления (Yield) из текущего protothread.
  *
  * Эта функция уступает процессорное время этого protothread, что позволяет
  * другим protothread выполняться в системе.
  *
  * \param pt Указатель на protothread control structure.
  *
  * \hideinitializer
  */
 #define PT_YIELD(pt)                            \
   do {                                          \
     PT_YIELD_FLAG = 0;                          \
     LC_SET((pt)->lc);                           \
     if(PT_YIELD_FLAG == 0) {                    \
       return PT_YIELDED;                        \
     }                                           \
   } while(0)
 
/**
  * \brief      Делает уступку процессорного времени из protothread, пока не
  *             не будет выполнено условие.
  * \param pt   Указатель на protothread control structure.
  * \param cond Условие.
  *
  *             Эта функция будет приводить к уступке процессорного времени 
  *             для этого protothread, пока указанное условие вычисляется
  *             как true.
  *
  *
  * \hideinitializer
  */
 #define PT_YIELD_UNTIL(pt, cond)                \
   do {                                          \
     PT_YIELD_FLAG = 0;                          \
     LC_SET((pt)->lc);                           \
     if((PT_YIELD_FLAG == 0) || !(cond)) {       \
       return PT_YIELDED;                        \
     }                                           \
   } while(0)
 
#define PT_YIELD_SPAWN(pt)			\
	do{								\
		PT_YIELD_FLAG = 0;			\
		LC_SET((pt)->lc);			\
		if (PT_YIELD_FLAG == 0){	\
			return PT_WAITING;		\
		}							\
	}while (0)

//#define PT_EXIT_OK(pt)			\
//	do{							\
//		PT_INIT(pt);			\
//		return PT_EXITED_OK;	\
//	}while (0)
//
//#define PT_EXIT_BAD(pt)			\
//	do{							\
//		PT_INIT(pt);			\
//		return PT_EXITED_BAD;	\
//	}while (0)
//
//#define PT_EXIT_TIMEOUT(pt)			\
//	do{								\
//		PT_INIT(pt);				\
//		return PT_EXITED_TIMEOUT;	\
//	}while (0)
//
//#define PT_EXIT_BREAK(pt)		\
//	do{							\
//		PT_INIT(pt);			\
//		return PT_EXITED_BREAK;	\
//	}while (0)
 /** @} */


 #endif /* __PT_H__ */

