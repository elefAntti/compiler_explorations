from functools import partial

#Approximate profunctor lenses in python.
#The types are really sloppy, thanks to pythons type system

#Profunctor Function
class Func:
	def __init__(self, fcn):
		self.run = fcn
	def dimap(self, f, g):
		return Func(lambda x: g(self.run(f(x))))
	def first(self):
		return Func(lambda pair: (self.run(pair[0]), pair[1]))
#Profunctor Forget
class Forget:
	def __init__(self, fcn = lambda x:x):
		self.run = fcn
	def dimap(self, f, g):
		return Forget(lambda x: self.run(f(x)))
	def first(self):
		return Forget(lambda pair: self.run(pair[0]))

def FieldLense(fieldname, pfunc):
	return pfunc.first().dimap(lambda state: (state[fieldname], state), lambda pair: { **pair[1], fieldname: pair[0] })

def Field(fieldname):
	return partial(FieldLense, fieldname)

def Setter(value):
	return Func(lambda x: value)

def Getter():
	return Forget()

def compose(f,g):
	return lambda x: f(g(x))

#Begin demo:

state = {"counter":0}

pfunc_add = Func(lambda x: x + 1)
counter_lense = Field("counter")

#pfunc_add, which is Function Int Int gets mapped to Function State State by the lense
add_counter = counter_lense(pfunc_add)

print("Initial:")
print(state)
print("After:")
new_state = add_counter.run(state)
print(new_state)
new_state = add_counter.run(new_state)
print(new_state)

get_counter = counter_lense(Getter())
print("Counter value: {}".format(get_counter.run(new_state)))

set_counter_to_5 = counter_lense(Setter(5))
new_state = set_counter_to_5.run(new_state)
print(new_state)

##Composition
state = {"substate":{"counter": 10}}
substate_lense = Field("substate")
print("Initial:")
print(state)
print("After:")

add_counter = substate_lense(counter_lense(pfunc_add))
new_state = add_counter.run(state)
print(new_state)

combined_lense = compose(substate_lense, counter_lense)

new_state = combined_lense(pfunc_add).run(new_state)
print(new_state)


## Prints the following

# Initial:
# {'counter': 0}
# After:
# {'counter': 1}
# {'counter': 2}
# Counter value: 2
# {'counter': 5}
# Initial:
# {'substate': {'counter': 10}}
# After:
# {'substate': {'counter': 11}}
# {'substate': {'counter': 12}}