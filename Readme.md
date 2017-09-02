# QKey

Based heavily on ideas from FinalKey[http://finalkey.net/], which is awesome
by the way. I made a FinalKey for myself a few years ago and have loved it.
Unfortunately its usb connector broke off the board at one point, which left
me in a bad spot. No backups; needing to reset all my passwords. So I made my
own, based on ideas I'd gathered.

## Design goals
* Easy backups -- Copy a file from the sd card
* Backups can easily unwrapped on a PC to migrate to another system
* Plausible deniability -- Hide clues if someone had guessed the master password.
* Easier menuing
* Fun for me to write

## Tradeoffs:
* I don't use AES, rather a weaker encryption algorithm which fits in the
  Atmel better.
* No multi-language support

## License

Licensed under the MIT license. Feel free to use this code for whatever, just
don't blame me if something goes wrong.

