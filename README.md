# HiML
HiML is a JSON like data format for serialization.

```
//comment
name = form
Pane {
    x = 1
    y = 2
    Button {
        id = button1
        x=1
        y=2
    }
    Button {
        x = 1, y = 2
    }
    1,2,3,
    a
    b
    c
}
```

# Key Idea
- no array, object as array
- no data type, metadata in mind
- quotes is options
- comma is options
- no braces for top level object
- comment by //
