#lfproto

lfproto will show you more infos about a lua function

# Features

* lfproto.line : linedefined and lastlinedefined
* lfproto.upvalues : upvalue names
* lfproto.source : source of function or filename
* lfproto.nparams : count of params
* lfproto.is_vararg : `...` 
* lfproto.paralist : params for this function
* lfproto.protos : function defined in this function
* lfproto.locals : all local variables, name and defined or destroy line number

# Build

you need some lua source such as lstate.h lobject.h lfunc.h


