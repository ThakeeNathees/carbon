

def generate_native_type_api(target, ctx): ## {name : src...}
	gen = "";
	for name in ctx.keys():
		file = open(ctx[name], 'r')
		gen += "const char* %s = \n" % name
		for line in file.readlines():
			gen += '"' + line.rstrip().replace('"', '\\"') + '\\n"\n'
		gen += ";\n\n"
		file.close();

	with open(target, 'w') as file:
		file.write(gen)


import sys
if __name__ == "__main__":
	target = sys.argv[1]
	ctx = eval(sys.argv[2])
	if type(ctx) != type(dict()):
		print("[nativeapi.gen] invalid argument")
	else:
		generate_native_type_api(target, ctx)