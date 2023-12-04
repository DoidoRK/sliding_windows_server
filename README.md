# UDP Sliding Window Protocol Server

## Overview

This UDP Sliding Window Protocol Server facilitates reliable file transfer between clients and the server using a sliding window approach. It supports both uploading and downloading files.

## Usage

### Compilation

Compile the server using:
```
make
```

### Running the Server

To run the client, use:
```
make run
```

## Important Notes

- Adjust `SERVER_PORT` constant in the source code according to your server's configuration.
- Uploaded files are placed in the server's destination directory: `src/files`.

## Dependencies

- C++ compiler (g++).
- `make` utility.

## Contributing

Feel free to contribute by opening issues, suggesting improvements, or submitting pull requests.

## License

This project is licensed under the [MIT License](LICENSE).
