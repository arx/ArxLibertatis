
def qdump__Vec2f(d, value):
    x = float(value['x'])
    y = float(value['y'])
    valueStr = "{:10.4f} {:10.4f}".format(x, y)
    d.putType("Vec2f")
    d.putValue(valueStr)
    d.putPlainChildren(value)

def qdump__Vec3f(d, value):
    x = float(value['x'])
    y = float(value['y'])
    z = float(value['z'])
    valueStr = "{:10.4f} {:10.4f} {:10.4f}".format(x, y, z)
    d.putType("Vec3f")
    d.putValue(valueStr)
    d.putPlainChildren(value)

def qdump__Anglef(d, value):
    yaw = float(value['m_yaw'])
    pitch = float(value['m_pitch'])
    roll = float(value['m_roll'])
    valueStr = "{:10.4f} {:10.4f} {:10.4f}".format(yaw, pitch, roll)
    d.putType("Anglef")
    d.putValue(valueStr)
    d.putPlainChildren(value)

def qdump__Color3f(d, value):
    r = float(value['r'])
    g = float(value['g'])
    b = float(value['b'])
    valueStr = "{:10.4f} {:10.4f} {:10.4f}".format(r, g, b)
    d.putType("Color3f")
    d.putValue(valueStr)
    d.putPlainChildren(value)


def qdump__Entity(d, value):
    d.putType("Entity")
    d.putValue(value['m_index'])
    d.putPlainChildren(value)

def qdump__EntityHandle(d, value):
    d.putType("EntityHandle")
    d.putValue(value['t'])
    d.putPlainChildren(value)

def qdump__LightHandle(d, value):
    d.putType("LightHandle")
    d.putValue(value['t'])
    d.putPlainChildren(value)
