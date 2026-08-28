# Pojobuf and Sajson

## Similarities

* `pojobuf::value` is almost identical to `sajson::value`. It is exactly identical in terms of members (two pointers and a type) and the only difference is the type tag. A sajson type tag is three bits, whereas a pojobuf type tag is four.
* The pojobuf JSON parser borrows large chunks of code from sajson. 
    * String parsing is (at this point) copied verbatim. 
    * Pojobuf uses `<charconv>` for numbers by default but a verbatim copy of sajson's number parsing is optionally available as well
    * The main parser goto-based state machine borrows heavily from sajson's.
* Like sajson, pojobuf allows parsing with a mutable source string as an external buffer in which case strings are used directly from the source string (and escapes are modified in-place to their exact byte values)

## Differences

* Values are clearly separated from the parser and builder
* Pojobuf JSON parsing can handle non-compound roots. In sajson the root node must be either an array or an object, whereas in pojobuf it can be any JSON value, say a number or a string
* More allocation and build strategies are available in pojobuf, notably putting the strings in the structure buffer, thus allowing a single buffer for a POJO.
* Pojobuf uses the front of the buffer, whereas sajson uses the back. This is helpful when one needs to pack multiple POJOs in the same buffer.
