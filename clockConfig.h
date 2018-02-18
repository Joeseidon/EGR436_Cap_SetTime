/*
 * clockConfig.h
 *
 *  Created on: Feb 4, 2018
 *      Author: joe
 */

#ifndef CLOCKCONFIG_H_
#define CLOCKCONFIG_H_

extern int getSMCLKfreq(void);

extern int getMCLKfreq(void);

extern void clockStartUp(void);

extern void clockInit48MHzXTL(void);

#endif /* CLOCKCONFIG_H_ */
