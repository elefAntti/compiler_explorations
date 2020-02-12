Profuntor Optics in Python
================================
Profunctor optics are a pattern in functional programming that generalize a field access (Lense) or cast (Prism) in Object oriented languages.
Thus Lense is a bit like getter/setter pair and a Prism is like downcast/upcast pair. There are also other possible optics.
THe cool thing about optics is that they compose: You can combine optics and you get optics.

Some time ago I read [Profunctor Optics: Modular Data Accessors](https://arxiv.org/abs/1703.10857) wich does good job of explaining the profunctor optics.
But I was curious if that pattern could be impleented in some other language that I know better, like Python.

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
