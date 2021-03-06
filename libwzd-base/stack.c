/*
 * wzdftpd - a modular and cool ftp server
 * Copyright (C) 2002-2008  Pierre Chifflier
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * As a special exemption, Pierre Chifflier
 * and other respective copyright holders give permission to link this program
 * with OpenSSL, and distribute the resulting executable, without including
 * the source code for OpenSSL in the source distribution.
 */

#include <stdlib.h>

#include "list.h"
#include "stack.h"

/****************************************************************
 *                                                              *
 *  ----------------------- stack_push -----------------------  *
 *                                                              *
 ***************************************************************/

int stack_push(Stack *pile, const void *donnee) {
return list_ins_next(pile, NULL, donnee);
}

/****************************************************************
 *                                                              *
 *  ----------------------- stack_pop ------------------------  *
 *                                                              *
 ***************************************************************/

int stack_pop(Stack *pile, void **donnee) {
return list_rem_next(pile, NULL, donnee);
}
