Automatic code generation for C/C++
====================================

First parse the source code file with libclang:

~~~
import clang.cindex
from mako.template import Template

class Enum:
	def __init__(self):
		self.namespace =[]
		self.name = ""
		self.cases = []

def collectEnums( node, namespace = [] ):
	if node.kind == clang.cindex.CursorKind.ENUM_DECL:
		enum = Enum()
		enum.name = node.spelling
		enum.cases = [ x.spelling for x in node.get_children() ]
		enum.namespace = list( namespace )
		return [ enum ]
	else:
		if node.kind == clang.cindex.CursorKind.NAMESPACE:
			namespace = namespace + [ node.spelling ]
		enums = []
		for child in node.get_children():
			enums += collectEnums( child, namespace )
		return enums


clang.cindex.Config.set_library_path('/usr/local/Cellar/llvm/5.0.0/lib')
index = clang.cindex.Index.create()
tu = index.parse('data/enum.h', args = '-x c++ --std=c++11'.split() )

enums = collectEnums( tu.cursor )
tpl = Template(filename='enum.mako')
print tpl.render( enums=enums, module_name='CodegenExample' )
~~~

You can then use templates to render what ever code you want to generate:
~~~
% for e in enums:
const char* toString( ${ e.name } eValue )
{
	% for v in e.cases:
	case ${e.name}::${v}: return "${v}";
	% endfor
	default: return "${e.name}_Unknown";
}

% endfor
~~~
