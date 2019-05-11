#include <vector>
//#include <iostream>

//Sink is a function (T -> Bool), where Bool is true  if iteration should stop
//Source is a function ( T -> Bool ) -> Bool where bool indicates that the iteration 
//was terminated by sink returning true

template< typename List >
auto fromList(List list)
{
	return [list]( auto sink )
	{
		for( auto element: list )
		{
			if(sink( element )) 
			{
				return true;
			}
		}
		return false;
	};
}

template< typename List >
auto insertToList( List& list )
{
    return [&list](auto element)
    { 
    	list.push_back( element ); 
    	return false; 
    };
}

/*template< typename Stream >
auto writeToStream( Stream& stream )
{
    return [&stream](auto element)
    { 
    	stream << element << std::endl; 
    	return false; 
    };
}

template< typename Source >
void toCout( Source&& source )
{
    source(writeToStream(std::cout));
}*/

template< typename List, typename Source >
List toList( Source&& source )
{
    List list;
    source(insertToList(list));
    return list;
}

template< typename Pred, typename Source >
auto filter( Pred pred, Source source )
{
	return [source, pred]( auto sink )
	{
		return source( [pred, sink](auto element)
		{ 
			return pred(element) ? sink(element) : false; 
		});
	};
}

template< typename Fcn, typename Source >
auto map( Fcn fcn, Source source )
{
	return [source, fcn]( auto sink )
	{
		return source( [fcn, sink](auto element){ return sink(fcn(element)); } );
	};
}

//constructs a stream containing only element t
template< typename T> 
auto mreturn( T t )
{
	return [t](auto sink)
	{
		return sink(t);
	};
}

//Flatten a stream of streams
template< typename Source >
auto join(Source source)
{
	return [source](auto sink)
	{
		source([sink](auto inner_source){return inner_source(sink);});
	};
}

//An empty stream (neutral element of stream concatenation monoid)
inline auto mzero = []( auto ){ return false; };

//Concatenete streams (mplus of stream concatenation monoid)
template< typename Source1 >
auto concat( Source1 source1 )
{
	return source1;
}

template< typename Source1, typename... Sources >
auto concat( Source1 source1, Sources... sources )
{
	return [ source1, sources... ]( auto sink )
	{
		return source1(sink)
		||concat(sources...)(sink);
	};
}

template< typename Source >
auto take( int n, Source source )
{
	return [source, n]( auto sink ) mutable
	{
		return source([sink, n](auto element) mutable
			{ 
				if(n > 0)
				{
					n--;
		 			return sink(element);
				}
				return true;
			} 
		);
	};
}

template< typename Source >
auto skip( int n, Source source )
{
	return [source, n]( auto sink ) mutable
	{
		return source([sink, n](auto element) mutable
		{ 
			if(n > 0)
			{
				n--;
	 			return false;
			}
			return sink(element);
		});
	};
}

auto iota()
{
	return [](auto sink)
	{
		for(int i = 0;;i++)
		{
			if(sink(i))
			{
				return true;
			}
		}
		return false;
	};
}

auto range( int low, int high )
{
	return [low, high](auto sink)
	{
		for(int i = low;i<high;i++)
		{
			if(sink(i))
			{
				return true;
			}
		}
		return false;
	};
}

bool sink(int);
int main()
{
    //std::vector<int> foo = {1, 2, 3, 4};
    //auto bar = toList<std::vector<float>>
    //toCout(
       /* map([](int i){return (float)i + 0.5f;},
            filter([](int i){return i % 2 == 1;}, 
                concat(//fromList(foo),
                    mreturn(5),
                    take(10,skip(2, iota()))
                    )))(&sink);*/
    //);*/
    concat(mreturn(5),mreturn(10))(&sink);
    //toCout(take(10, iota()));
    /*for( auto elem: bar )
    {
        std::cout << elem << std::endl;
    }*/
}
