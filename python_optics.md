Profunctor Optics in Python
================================
Profunctor optics are a pattern in functional programming that generalize a field access (Lense) or cast (Prism) in object oriented languages. Lense and Prism are duals: One focuses on a part and the other on an alternative.
Thus Lense is a bit like getter/setter pair and a Prism is like downcast/upcast pair. There are also other possible optics.
The cool thing about optics is that they are just functions and thus they compose: You can combine optics and you get optics.

Some time ago I read [Profunctor Optics: Modular Data Accessors](https://arxiv.org/abs/1703.10857) wich does good job of explaining the profunctor optics.
But I was curious if that pattern could be implemented in some other language that I know better, like Python.

Pythons type-system makes this interesting, because normally these profunctor optics (and profunctors themselves) would be strongly typed.
Things will look a bit different with pythons ducktyping. I'm not diving deep into the category theory here either. So to get a more exact
view of what these optics are, check the original article.

Profunctors
----------------
Roughly speaking profunctors are for our purposes function like relationships between two types.
For example, functions as a profunctor would be something like

~~~
class Func:
	def __init__(self, fcn):
		self.run = fcn
	def dimap(self, f, g):
		return Func(lambda x: g(self.run(f(x))))
~~~

So it is something that you can construct from a regular python function and you can 'run' it, which is just the same as calling that original function.
But you can also dimap it with two functions. This is the essential part of it being a profunctor. The dimap makes another instance of the same profunctor type.
It takes the functions passed as a parameter and chains them with the 'run' function of the original profunctor object so that 
f operates on the input of that function and g on its output.

There are plenty of other possible profunctors, but here we define only one 

~~~
class Forget:
	def __init__(self, fcn = lambda x:x):
		self.run = fcn
	def dimap(self, f, g):
		return Forget(lambda x: self.run(f(x)))
~~~

The Forget profunctor is a lot like the Func profunctor above, but it's dimap just drops the g. Thus wen you run a Forget profunctor that was formed by dimapping another one,
it will return you the same type of value as the original profunctors 'run' would.

Optics
----------------
Let's first think about a simple type of optics: Adapter. There are two functions in an adapter, f and g. Those functions form an isomorphism: f(g(x)) = x and g(f(x)) = x. This optic can change the shape of data, for example ((a, b), c) to (a, b, c) before applying a function to get (h, i, j) and then change it back to ((h, i), j). It would map a profunctor simply by dimaping the g and f functions to it. One interesting optic of this type is changing the base of a vector space.

Lenses
-----------------
The Lense divides some object into a focused part and the rest. It then apples the profunctor to the focus and recombines it with the rest. For this the profunctor has to be cartesian: Having a profunctor from a to c we can apply the profunctor to (a, b) to get to (c, b) where those parentheses denote tuples. This is done by the member function "first":

~~~
class Func:
	def __init__(self, fcn):
		self.run = fcn
	def dimap(self, f, g):
		return Func(lambda x: g(self.run(f(x))))
	def first(self):
		return Func(lambda pair: (self.run(pair[0]), pair[1]))
		
class Forget:
	def __init__(self, fcn):
		self.run = fcn
	def dimap(self, f, g):
		return Forget(lambda x: self.run(f(x)))
	def first(self):
		return Forget(lambda pair: self.run(pair[0]))
~~~

There is a similar "second" function, but we don't need it here. Now a lense focusing on a certain field of a dict could be written as follows:

~~~
def FieldLense(fieldname, pfunc):
	return pfunc.first().dimap(lambda state: (state[fieldname], state), lambda pair: { **pair[1], fieldname: pair[0] })
~~~

So first we convert the profunctor to a profunctor acting on a (focus, the rest) pair, then we dimap functions of which first extracts the focus and the second substitutes it back. We now have a function that takes a profunctor and returns a profunctor. Note that I cheat and "the rest" is just the entire original object.

Lets define a few helpers and then do a demo

~~~
from functools import partial

def Field(fieldname):
	return partial(FieldLense, fieldname)
	
pfunc_add = Func(lambda x: x + 1)
counter_lense = Field("counter")

add_counter = counter_lense(pfunc_add)


state = {"counter":0}

print(state) 
# prints {'counter': 0}

new_state = add_counter.run(state)
print(new_state)
# prints {'counter': 1}

new_state = add_counter.run(new_state)
print(new_state)
# prints {'counter': 2}
~~~

So we have changed a function operating on integers to a function operating on dicts.

To illustrate how this relates to the concept of getter and setter functions (and also to explain what we need the Forget profunctor for) lets try the following:

~~~
def Setter(value):
	return Func(lambda x: value)

def Getter():
	return Forget(lambda x: x)


get_counter = counter_lense(Getter())
print("Counter value: {}".format(get_counter.run(new_state)))
# prints Counter value: 2

set_counter_to_5 = counter_lense(Setter(5))
new_state = set_counter_to_5.run(new_state)
print(new_state)
#prints {'counter': 5}
~~~

So we can recover getter and setter functions from the profunctor optics. The profunctor lense can also be made from getter & setter pair: they are isomorphic.

A brief demonstration about how you can compose two lenses or map the profunctor first trough one and then the other

~~~

def compose(f,g):
	return lambda x: f(g(x))
	
state = {"substate":{"counter": 10}}

substate_lense = Field("substate")

print(state)
# prints {'substate': {'counter': 10}}

add_counter = substate_lense(counter_lense(pfunc_add))
new_state = add_counter.run(state)
print(new_state)
# prints {'substate': {'counter': 11}}

combined_lense = compose(substate_lense, counter_lense)

new_state = combined_lense(pfunc_add).run(new_state)
print(new_state)
# prints {'substate': {'counter': 12}}
~~~


Prisms
----------
For the Lenses we used cartesian profunctors so we could make them operate on a tuple. Because Prisms are dual to Lenses, they use cocartesian profunctors: a profunctor that can be lifted over an "either" type. In python, unfortunately, any varible can contain any type and there is no separate "either". But we can emulate it by making these two types corresponding to the two constructors of an "either"

~~~
# Emulating the either type....
class Left():
	def __init__(self, value):
		self.value = value

class Right():
	def __init__(self, value):
		self.value = value
~~~

Luckily Func is also a cocartesian profunctor:

~~~
#Profunctor Function (both Cartesian & Cocartesian)
class Func:
	def __init__(self, fcn):
		self.run = fcn
	def dimap(self, f, g):
		return Func(lambda x: g(self.run(f(x))))
	def first(self):
		return Func(lambda pair: (self.run(pair[0]), pair[1]))
	def left(self):
		return Func(lambda value: Left(self.run(value.value)) if type(value) is Left else value)
~~~

the "left" function maps the profunctor to a new profunctor that lets a "Right" value through unmodified, but operates on the "Left" value. There would also be a "right" function, but we don't need it now.

Many parsers can be seen as prisms and the example is a prism that parses floats and converts them back to strings. It focuses on the alternative of the string being a representation of a float.

~~~
def try_parse(value):
	try:
		return Left(float(value))
	except ValueError:
		return Right(value)

#parser prism between str and float
def float_prism(pfunc):
	return pfunc.left().dimap(try_parse, lambda value: str(value.value) )
~~~

Lets demonstrate this

~~~
pfunc_double = Func(lambda x: x * 2.0)

double_str = float_prism(pfunc_double)

print("1.2 doubled: {}".format(double_str.run("1.2")))
# prints 1.2 doubled: 2.4
print("kissa doubled: {}".format(double_str.run("kissa")))
# prints kissa doubled: kissa
~~~

You can also combine lenses with prisms

~~~
str_counter = compose(counter_lense, float_prism)

state = {"counter": "3.14"}

new_state = str_counter(double_str).run(state)

print(state)
# prints {'counter': '3.14'}
print(new_state)
# prints {'counter': '6.28'}
~~~
