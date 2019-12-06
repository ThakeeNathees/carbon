from enum import Enum

class Stack:
	def __init__(self):
		self.stack = []
	def is_empty(self):
		return len(self.stack) == 0
	def push(self, value):
		self.stack.append(value)
	def pop(self):
		if (self.is_empty()):
			raise Exception('Error: stack under flow')
		return self.stack.pop()
	def peek(self, pos=-1):
		if (self.is_empty()):
			raise Exception('Error: stack under flow')
		return self.stack[pos]

def is_operator(char):
	return char in list('^+-*/')

def is_func(string):
	return string in ['min', 'max', 'sin']

def get_precedence(char):
	if char in ('+', '-')  : return 1
	elif char in ('*', '/'): return 2
	elif char == '^'       : return 3
	elif char in ('(',')') : return 0
	elif is_func(char)     : return 0
	else: raise Exception('Error: invalid operator to get percedence %s' % char)



#max ( 1 + 2 , 3 )
def main():
	infix = input('infix> ').split()
	operator_stack = Stack()
	operand_stack  = Stack()

	infix.append(')')
	operator_stack.push('(')

	for char in infix:
		if char == '(':
			operator_stack.push(char)

		elif is_func(char):
			operator_stack.push(char)

		elif char.isalpha() or char.isdigit():
			operand_stack.push(char)

		elif is_operator(char):
			top = operator_stack.pop()
			while (is_operator(top) or is_func(top)) and (get_precedence(char) <= get_precedence(top) ):
				operand_stack.push(top)
				top = operator_stack.pop()
			operator_stack.push(top)
			operator_stack.push(char)

		elif char == ')':
			top = operator_stack.pop()
			while top != '(':
				operand_stack.push(top)
				top = operator_stack.pop()
			## if the next top (if exists) is func apply
			if not operator_stack.is_empty() and is_func(operator_stack.peek()):
				operand_stack.push( operator_stack.pop() )

		## like ')' but not pop and ignore '('
		elif char == ',':
			top = operator_stack.pop()
			while top != '(':
				operand_stack.push(top)
				top = operator_stack.pop()
			operator_stack.push(top)
		else:
			raise Exception('Error: unknown char %s' % char)

	if not operator_stack.is_empty():
		raise Exception('Error: invalid expression %s', infix[:-1])

	print(' '.join(operand_stack.stack))

if __name__ == '__main__':
	main()