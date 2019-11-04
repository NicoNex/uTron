/*
 * uTron
 * Copyright (C) 2019  Nicolò Santamaria
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef BTREE_H_
#define BTREE_H_

#include "bot.h"

struct node {
	int64_t id;
	struct bot *bot;
	struct node *left;
	struct node *right;
};

typedef struct node *btree;

struct node *new_node(int64_t id, struct bot *bot);
void add_node(btree *root, struct node *node);
struct node *get_node(btree root, int64_t id);
void del_node(btree *rootaddr, int64_t id);


#endif // BTREE_H_
