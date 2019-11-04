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

#include <stdlib.h>
#include "btree.h"
#include "bot.h"

struct node *new_node(int64_t id, struct bot *bot) {
	struct node *node = calloc(1, sizeof(struct node));
	node->id = id;
	node->bot = bot;
	return node;
}

void add_node(struct node **rootaddr, struct node *node) {
	if (*rootaddr != NULL) {
		struct node *current = *rootaddr;
		add_node(node->id < current->id ? &current->left : &current->right, node);
	}
	else
		*rootaddr = node;
}

struct node *get_node(struct node *root, int64_t id) {
	if (root != NULL) {
		if (id == root->id)
			return root;

		return get_node(id < root->id ? root->left : root->right, id);
	}
	return NULL;
}

static void dispose_node(struct node *root) {
	if (!root)
		return;
	if (root->bot)
		free(root->bot);
	free(root);
}

void del_node(struct node **rootaddr, int64_t id) {
	struct node *current = *rootaddr;
	if (current) {
		if (current->id == id) {
			struct node *tmp;

			if (current->left && current->right) {
				tmp = current->left;
				add_node(&current->left, current->right);
			}
			else if (current->left || current->right)
				tmp = current->left ? current->left : current->right;
			else
				tmp = NULL;

			*rootaddr = tmp;
			dispose_node(current);
		}
		else
			del_node(id < current->id ? &current->left : &current->right, id);
	}
}
