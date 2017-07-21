
def qdump__Vec2f(d, value):
    x = value['#1']['x'].value()
    y = value['#2']['y'].value()
    valueStr = "{:10.4f} {:10.4f}".format(x, y)
    
    d.putType("Vec2f")
    d.putValue(valueStr)
    d.putPlainChildren(value)

def qdump__Vec3f(d, value):
    x = value['#1']['x'].value()
    y = value['#2']['y'].value()
    z = value['#3']['z'].value()
    valueStr = "{:10.4f} {:10.4f} {:10.4f}".format(x, y, z)
    
    d.putType("Vec3f")
    d.putValue(valueStr)
    d.putPlainChildren(value)

def qdump__Anglef(d, value):
    yaw   = value['m_yaw'].value()
    pitch = value['m_pitch'].value()
    roll  = value['m_roll'].value()
    valueStr = "{:10.4f} {:10.4f} {:10.4f}".format(yaw, pitch, roll)
    
    d.putType("Anglef")
    d.putValue(valueStr)
    d.putPlainChildren(value)

def qdump__Color3f(d, value):
    r = value['r'].value()
    g = value['g'].value()
    b = value['b'].value()
    valueStr = "{:10.4f} {:10.4f} {:10.4f}".format(r, g, b)
    
    d.putType("Color3f")
    d.putValue(valueStr)
    d.putPlainChildren(value)


def qdump__Entity(d, value):
    index = value['m_index'].value()
    
    d.putType("Entity")
    d.putValue("Idx: " + str(index))
    d.putPlainChildren(value)

def qdump__EntityHandle(d, value):
    val = value['t'].value()
    
    d.putType("EntityHandle")
    d.putValue(val)
    d.putPlainChildren(value)

def qdump__LightHandle(d, value):
    val = value['t'].value()
    
    d.putType("LightHandle")
    d.putValue(val)
    d.putPlainChildren(value)
