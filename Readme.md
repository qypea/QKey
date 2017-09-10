# QKey

Based heavily on ideas from FinalKey[http://finalkey.net/], which is awesome
by the way. I made a FinalKey for myself a few years ago and have loved it.
Unfortunately its usb connector broke off the board at one point, which left
me in a bad spot. No backups; needing to reset all my passwords. So I made my
own, based on ideas I'd gathered.

## Design goals
* Easy backups -- Copy a file from the sd card
* Backups can be easily unwrapped on a PC to migrate to another system
* Slow to brute force -- see below
* Easier menuing
* Fun for me to write

## Tradeoffs:
* I don't use AES, rather a weaker encryption algorithm which fits in the
  Atmel better.
* No multi-language support

## Slow to brute force

One of the issues I see with password vaults is that they tend to be a gold
mine that can be used to break the cipher. There is a lot of metadata in
specific formats under the cipher, so someone can take the data and use that
knowledge to brute force the master password and know when they've got it
right. Even if the cipher just covers the password, that can be used to throw
out most bad decryptions since its only using some characters and ends with a
null. This project uses a huffman compression and random padding to ensure that
someone decrypting this file has very few clues if they've decoded it properly
until they send a password to a website. This will hopefully make it harder to
crack, because accessing a website may alert me or lock that account out.

A side-effect of this is that instead of telling the user if they put in the
master password correctly I have to show a random token which is encrypted.
This serves as a challenge; if it doesn't match what you remember then you
mistyped the password.

## Hardware

To run this code you'll need a specific set of hardware
* Adafruit Feather 32u4 Adalogger[https://www.adafruit.com/product/2795]
* SD card
* USB cable/adapter
* Micro pushbutton switch(Solder between pins 10 and 12)

## License

Licensed under the MIT license. Feel free to use this code for whatever, just
don't blame me if something goes wrong.

