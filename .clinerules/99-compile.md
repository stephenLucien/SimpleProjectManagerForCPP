- project compilation is maintained by bash script `${workspaceFolder}/compile.sh`, which had been created and tested, just use it.
  - project build: `${workspaceFolder}/compile.sh cline_build`
  - project run: `${workspaceFolder}/compile.sh run`

- after compilation, project `run` to test the project code

- since the project is lightweight, prefer to build the project whenever you feel good about the code.
- Fix code only after compilation errors, LSP server may also help you address the problems.

- LSP server may complain `header` file problems, just ignore it.
