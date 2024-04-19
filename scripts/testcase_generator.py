import random
import itertools
import string

test_case_type = 3
num_clients = 10
max_orders = 10000
max_instruments = 500



def buy_sell_race():
    print(num_clients)
    print("o")
    order_num = 1
    for i in range (max_orders):
        for j in range (num_clients):
            #fill order book with non matching instructions
            print(f'{j} B {order_num} KEKW 2 1')
            order_num+=1
            print(f'{(j+1)%num_clients} S {order_num} KEKW 10 8888')
            order_num+=1
            print(f'{j} S {order_num} HUH 10 8888')
            order_num+=1
            print(f'{(j+1)%num_clients} B {order_num} HUH 2 1')
            order_num+=1
            # matching instructions
            client1 = random.randint(0, num_clients - 1)
            client2 = random.randint(0, num_clients - 1) 
            print(f'{client1} B {order_num} KEKW 700 700')
            print(f'{client1} w {order_num}')
            order_num+=1
            print(f'{client2} S {order_num} KEKW 700 700')
            print(f'{client2} w {order_num}')
            order_num+=1
            print(f'{client2} B {order_num} HUH 700 700')
            print(f'{client2} w {order_num}')
            order_num+=1
            print(f'{client1} S {order_num} HUH 700 700')
            print(f'{client1} w {order_num}')
            order_num+=1
    print("x")
    

def cancel_race():
    print(num_clients)
    print("o")
    order_num = 1
    for i in range (max_orders):
        for j in range (num_clients):
            print(f'{j} B {order_num} HUH 700 700')
            print(f'{j} C {order_num}')
            order_num+=1
            print(f'{(j+1)%num_clients} S {order_num} HUH 700 700')
            print(f'{(j+1)%num_clients} C {order_num}')
            order_num+=1
    print("x")

def match_multiple_orders():
    print(num_clients)
    print("o")
    order_num = 1
    for i in range (max_orders):
        for j in range (num_clients-1):
            print(f'{j} S {order_num} HUH 700 2')
            order_num += 1
        print(f'{num_clients-1} B {order_num} HUH 700 {2*(num_clients-1)}')
        order_num+=1
    print("x")

def match_best_price():
    print(num_clients)
    print("o")
    order_num = 1
    for i in range (max_orders):
        for j in range (num_clients-1):
            print(f'{j} S {order_num} HUH {num_clients - j} 1')
            order_num += 1
        print(f'{num_clients-1} B {order_num} HUH 700 {(num_clients-1)}')
        order_num+=1
    print("x") 

def random_orders():
    instruments = []
    orders = []
    for i in range (num_clients):
        orders.append([])
    strings_generated = 0
    for length in range(1, 9):
        for combination in itertools.product(string.ascii_uppercase, repeat=length):
            instruments.append(''.join(combination))
            strings_generated += 1
            if strings_generated == max_instruments:
                break
        if strings_generated == max_instruments:
            break
    print(num_clients)
    print("o")
    order_num = 1
    for i in range(max_orders * max_instruments):
        client = random.randint(0, num_clients - 1)
        type = random.randint(0, 2)
        instr = random.randint(0, max_instruments-1)
        price = random.randint(1, 7777)
        amt = random.randint(1, 7777)
        if(type == 1):
            print(f'{client} B {order_num} {instruments[instr]} {price} {amt}')
            orders[client].append(order_num)
            order_num+=1
        elif(type == 2):
            print(f'{client} S {order_num} {instruments[instr]} {price} {amt}')
            orders[client].append(order_num)
            order_num += 1
        else:
            if(len(orders[client]) == 0):
                continue
            num = random.randint(0, len(orders[client]) - 1)
            print(f'{client} C {orders[client][num]}')
    print("x")

def main():
    if(test_case_type == 0):
        buy_sell_race()
    elif(test_case_type == 1):
        match_multiple_orders()
    elif(test_case_type == 2):
        cancel_race()
    elif(test_case_type == 3):
        match_best_price()
    else:
        random_orders()

if __name__ == '__main__':
    main()