; ModuleID = 'BangCompiler'

declare void @printf(i8*, ...)

define i32 @main() {
entry:
  %x = alloca i32
  store i32 5, i32* %x
  ret i32 0
}
