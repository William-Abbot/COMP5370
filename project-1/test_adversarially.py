from serialization import marshal
from deserialization import unmarshal
from exceptions import SerializationError, DeserializationError

def test_case_a():
    # Space within a string should remain while outside the string should be removed
    got = unmarshal(marshal({'key' :      'one two'       }))
    assert(got == {'key':'one two'})

def test_case_b():
    #String contains a comma that is not percent encoded
    try:
        got = unmarshal('{pet:cat,dog,apple:pears}')
    except Exception as e:
        assert(isinstance(e, DeserializationError))

def test_case_c():
    #String gets interpretted as int
    try:
        got = unmarshal('{a:ibcd}')
    except Exception as e:
        assert(isinstance(e, DeserializationError))

def test_case_d():
    #Percent sign that does not represent a percent encoded characte
    try:
     got = unmarshal('{a:ab%aa}')
    except Exception as e:
        assert(isinstance(e, DeserializationError))

def test_case_e():
    #Nested Map contains empty strings unchecked
    try:
     got = marshal({'a' : {'b' : 'boy' , '' : 'cat'}})
    except Exception as e:
        assert(isinstance(e, SerializationError))