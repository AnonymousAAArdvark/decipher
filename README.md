# decipher
![demo](https://github.com/AnonymousAAArdvark/decipher/blob/master/demo.gif)  
This CLI app implements a Hollywood style decryption effect seen in many movies.
Decipher takes output from piped data and applies the effect by first showing the "encrypted" data, and then decripting it to show the original message.

## dependencies
This project does not need any dependencies. However, it will only work on VT100 supported terminals for the ability to rewrite multiple lines.

## download and install
### install:
```
$ git clone https://github.com/AnonymousAAArdvark/decipher.git
$ cd ./decipher
$ g++ main.cpp -o decipher
$ cp decipher /usr/local/bin
```
### uninstall:
```
$ rm -rf /usr/local/bin/decipher
```
## usage
```
Example:
  ls -l / | decipher

Usage:
  ls -l \ | decipher -s <speed>  Set decryption speed (higher is faster)
  ls -l \ | decipher -a          Set auto decipher flag
  ls -l \ | decipher -w          Encrypt whitespace flag
  ls -l \ | decipher -c <color>  Set solved color flag
  ls -l \ | decipher -h          display this help message
```
