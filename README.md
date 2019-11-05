# uTron

Engine for Telegram bot written in pure c.

## Usage

A very simple implementation is:

```c
#include "utron/utron.h"

struct bot {
    int64_t chat_id;
};

struct bot *new_bot(int64_t chat_id) {
    struct bot \*b = malloc(sizeof(struct bot));
    b->chat_id = chat_id;
    return b;
}

void update_bot(struct bot *bot, struct json_object *update) {
    tg_send_message("Hello World!", bot->chat_id);
}

int main() {
    run_dispatcher("YOUR TELEGRAM API TOKEN");
}
```
