## EvalHook PHP Extension for Code Decryption

### The original article

http://php-security.org/2010/05/13/article-decoding-a-user-space-encoded-php-script/index.html

### My Modification

- Remove cli interaction for web server integration (no execution confirmation, use with caution)
- Dump decrypted code to seperate files (`*.dec.php` in the same directory of the original file)
- Can prevent duplicated dumps

### Usage

```
phpize 
./configure 
make && sudo make install
sudo service apache2 restart  # (or equivalence for other servers) 
```
