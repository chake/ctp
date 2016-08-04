#!/bin/bash

# nohup php tools/tick.php &
nohup php tools/order.php &
nohup php tools/kLine.php &
nohup php tools/push2db.php &
