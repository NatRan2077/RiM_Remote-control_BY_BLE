
/*
 * Copyright (c) 2004, Swedish Institute of Computer Science.
 * All rights reserved. 
 *
 * Redistribution and use in source and binary forms, with or without 
 * modification, are permitted provided that the following conditions 
 * are met: 
 * 1. Redistributions of source code must retain the above copyright 
 *    notice, this list of conditions and the following disclaimer. 
 * 2. Redistributions in binary form must reproduce the above copyright 
 *    notice, this list of conditions and the following disclaimer in the 
 *    documentation and/or other materials provided with the distribution. 
 * 3. Neither the name of the Institute nor the names of its contributors 
 *    may be used to endorse or promote products derived from this software 
 *    without specific prior written permission. 
 *
 * THIS SOFTWARE IS PROVIDED BY THE INSTITUTE AND CONTRIBUTORS ``AS IS'' AND 
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE 
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE 
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE 
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL 
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS 
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) 
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT 
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY 
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF 
 * SUCH DAMAGE. 
 *
 * This file is part of the protothreads library.
 * 
 * Author: Adam Dunkels <adam@sics.se>
 *
 * $Id: pt-sem.h,v 1.2 2005/02/24 10:36:59 adam Exp $
 */

/**
 * \addtogroup pt
 * @{
 */

/**
 * \defgroup ptsem Protothread semaphores
 * @{
 *
 * Этот модуль определяет счетные семафоры для портопотоков. Семафоры 
 * Семафоры это средства синхронизации которые обеспечивают две операции:
 * : "ждать" и "выдать". Операция "ждать" проверяет счетчик семафора
 * и блокирует поток если счетчик равен нулю. Операция "выдать" увеличивает  
 * счетчик семафора но не блокирует его. Если другой поток блокирован 
 * ожиданием семафора который выдан, то он разблокируется.
 *
 * Семафоры могут быть использованы чтобы выполнять другие, более структурированные ,
 * примитивы синхронизации такние как мониторы и сообщения 
 * queues/bounded buffers (смотри ниже).
 *
 * Следующий пример показывает как производитель - потребитель, также 
 * известное как проблема bounded buffer problem, может быть решена через использование 
 * протопотоков и семафоров. Заметки по пограмме следуют после примера 
 *
 \code
#include "pt-sem.h"

#define NUM_ITEMS 32
#define BUFSIZE 8

static struct pt_sem mutex, full, empty;

PT_THREAD(producer(struct pt *pt))
{
  static int produced;
  
  PT_BEGIN(pt);
  
  for(produced = 0; produced < NUM_ITEMS; ++produced) {
  
    PT_SEM_WAIT(pt, &full);
    
    PT_SEM_WAIT(pt, &mutex);
    add_to_buffer(produce_item());    
    PT_SEM_SIGNAL(pt, &mutex);
    
    PT_SEM_SIGNAL(pt, &empty);
  }

  PT_END(pt);
}

PT_THREAD(consumer(struct pt *pt))
{
  static int consumed;
  
  PT_BEGIN(pt);

  for(consumed = 0; consumed < NUM_ITEMS; ++consumed) {
    
    PT_SEM_WAIT(pt, &empty);
    
    PT_SEM_WAIT(pt, &mutex);    
    consume_item(get_from_buffer());    
    PT_SEM_SIGNAL(pt, &mutex);
    
    PT_SEM_SIGNAL(pt, &full);
  }

  PT_END(pt);
}

PT_THREAD(driver_thread(struct pt *pt))
{
  static struct pt pt_producer, pt_consumer;

  PT_BEGIN(pt);
  
  PT_SEM_INIT(&empty, 0);
  PT_SEM_INIT(&full, BUFSIZE);
  PT_SEM_INIT(&mutex, 1);

  PT_INIT(&pt_producer);
  PT_INIT(&pt_consumer);

  PT_WAIT_THREAD(pt, producer(&pt_producer) &
		     consumer(&pt_consumer));

  PT_END(pt);
}
 \endcode
 *
 * Эта программа использует три протопотока: один протопоток который 
 * выполняет функцию потребителя, один поток который выполняет функцию ,
 * производителя и один поток который управляет двумя другими протопотоками.
 * Программа использует три семафора: "полный", "пустой" и "мьютекс". 
 * Семафор "мьютекс" используется чтобы обеспечить общее выполнение для 
 * буффера, "пустой" семафор используется чтобы блокировать потребителя если 
 * буффер пуст, и "полный" семафор используется чтобы блокировать 
 * производителя если буффер полон.
 *
 * Протопоток "driver_thread" хранит две переменных - состояния портопотока,
 * "pt_producer" и "pt_consumer". Важно заметить что обе эти переменные 
 * описаны как <i>static</i>. Если ключевое слово "static" не использовать,
 * обе переменные сохраняются в стеке. Так как портопотоки не сохраняются в стеке,
 * эти переменные могут быть перезаписаны в течении выполнения ожидания протопотоком
 * семафора. Аналогично, оба "потребилель" и "производитель" объявляют локальные
 * переменные как static, чтобы избежать сохранения их в стеке программы.
 * 
 *
 */
   
/**
 * \file
 * Счетные семафоры выполнены на протопотоках
 * \author
 * Adam Dunkels <adam@sics.se>
 *
 */

#ifndef __PT_SEM_H__
#define __PT_SEM_H__

#include "pt.h"

typedef struct {
  unsigned int count;
}pt_sem_t;

/**
 * Инициализация семафора
 *
 * Этот макром инициализирует семафор. Этот макрос инициализирует семафор в значение 
 * counter. Семафор непосредственно использует "unsigned int" чтобы 
 * represent the counter, и следовательно аргумент "count" должен быть 
 * в диапазоне unsigned int.
 *
 * \param s (struct pt_sem *) Указатель на структуру pt_sem struct
 * представляющую семафор 
 *
 * \param c (unsigned int) Начальное значение счетчика семафора.
 * \hideinitializer
 */
#define PT_SEM_INIT(s, c) (s)->count = c

/**
 * Ждем семафор
 *
 * Этот макрос выполняет операцию "ожидание" семафора. Операция 
 * ожидания блокирует протопоток пока счетчик не станет равен нулю
 * Когда счетчик достигает значения большего, чем ноль,
 * протопоток продолжит выполняться .
 *
 * \param pt (struct pt *) Указатель на портопоток (struct pt) в 
 * котором выполняется операция.
 *
 * \param s (struct pt_sem *) Указатель на структуру pt_sem struct
 * соответствующую семафору
 *
 * \hideinitializer
 */
#define PT_SEM_WAIT(pt, s)	\
  do {						\
    PT_WAIT_UNTIL(pt, (s)->count > 0);		\
    --(s)->count;				\
  } while(0)

/**
 * Выдача семафора 
 *
 * Этот макрос выдает семафор. Выдача семафора инкрементирует счетчик 
 * внутри семафора, который в конечном счете будет причиной
 * продолжения выполнения портопотока.
 *
 * \param pt (struct pt *) Указатель на портопоток (struct pt) в 
 * котором выполняется операция.
 *
 * \param s (struct pt_sem *) Указатель на структуру pt_sem struct
 * соответствующую семафору
 *
 * \hideinitializer
 */
#define PT_SEM_SIGNAL(pt, s) ++(s)->count

#endif /* __PT_SEM_H__ */

/** @} */
/** @} */
   
