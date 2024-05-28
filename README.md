# RandomDev

Simple Linux character device — integer generator based on constant-recursive sequences of bytes

## Simple Usage

To use the RandomDev, follow these steps:

1. Clone the repository:

    ```
    git clone https://github.com/PolinaSavelyeva/RandomDev.git
    ```

2. Navigate to the RandomDev directory:

    ```
    cd RandomDev
    ```

3. Build the library using Make:

    ```
    make
    ```
    
4. Insert module and log kernel messages:
    ```
    sudo insmod mymodule.ko
    ```
    ```
    sudo dmesg
    ```
    
    or use:
    ```
    make test
    ```
    
    In kernel log you can see `MAJOR` number for your future character device.

5. Assign this `MAJOR` number to a new character device:
    ```
    sudo mknod /dev/yourdevice c MAJOR 0
    ```
    
6. Make your device file writable and put data into it using `printf`:
    ```
    sudo chmod a+w /dev/yourdevice
    ```
    ```
    printf "\K\A_1\...\A_K\X_1\...\X_K\C" > /dev/yourdevice
    ```
    where,
    - `K` is an amount of coefficients to be provided in hexadecimal
    - `A_i` and `X_i` are hexadecimal coefficients, 1<=i<=K
    -  `C` is a last polynomial hexadecimal coefficient
    
    example: 
    `K` = 3, 
    `A_1` = 5, `A_2` = 2, `A_3` = 1, 
    `X_1` = 3, `X_2` = 3, `X_3` = 7, 
    `C` = 4
    ```
    printf "\x03\x05\x02\x01\x03\x03\x07\x04" > /dev/yourdevice
    ```

7. Read a sequence of random numbers using `xxd`:
    ```
    xxd /dev/yourdevice
    ```
    
8. Remove device file:
    ```
    sudo rm /dev/yourdevice
    ```

9. Remove module from kernel:
    ```
    sudo rmmod mymodule
    ```

## Build Requirements

- Make utility
- C compiler

Tested on linux kernel version 5.15.0-107-generic.
