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

// This struct defines each node in the binary search tree.
struct node {
	int64_t id;
	struct bot *bot;
	struct node *left;
	struct node *right;
};

typedef struct node *btree;

// This function allocates a new node and returns its address.
struct node *new_node(int64_t id, struct bot *bot);

// This function adds a given node to a binary tree given its root
// in input.
void add_node(btree *root, struct node *node);

// Use this function to retrieve the node in the binary tree that has
// the corresponding id specified in input.
struct node *get_node(btree root, int64_t id);

// Use this function to delete the node from the binary tree that has
// the corresponding id specified in input.
void del_node(btree *rootaddr, int64_t id);

#endif // BTREE_H_
