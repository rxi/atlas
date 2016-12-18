# atlas
A small utility for generating a texture atlas from all the images and truetype
fonts in a directory.


## Basic usage
```bash
atlas assetdir
```
This will pack the images and fonts from `assetdir` and create the file
`out.png`. `out.txt` will also be created, containing a list of all the packed
images and their locations.

The `-f` argument can be used to specify a custom line format for the text file,
for example, the following might be used for XML:
```bash
atlas -f "<image name='%s' x='%d' y='%d' width='%d' height='%d'/>" assetdir
```

Run `atlas --help` for a list of all possible arguments.


## License
This project is free software; you can redistribute it and/or modify it under
the terms of the MIT license. See LICENSE for details.

The project uses several of the [stb libraries](https://github.com/nothings/stb)
which are in the public domain.
