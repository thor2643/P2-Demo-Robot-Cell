import math
import numpy as np

lenght = 35/90*25.80
print(lenght)
radius = math.sqrt(pow(361.905,2)+ pow(-281.078,2))

print(radius)

pos1 = [361.905, -281.078]
pos2 = [447.892, -96.838]

def unit_vector(vector):
    """ Returns the unit vector of the vector.  """
    return vector / np.linalg.norm(vector)

def angle_between(v1, v2):
    """ Returns the angle in radians between vectors 'v1' and 'v2'::

            >>> angle_between((1, 0, 0), (0, 1, 0))
            1.5707963267948966
            >>> angle_between((1, 0, 0), (1, 0, 0))
            0.0
            >>> angle_between((1, 0, 0), (-1, 0, 0))
            3.141592653589793
    """
    v1_u = unit_vector(v1)
    v2_u = unit_vector(v2)
    return np.arccos(np.clip(np.dot(v1_u, v2_u), -1.0, 1.0))

angle = angle_between(pos1, pos2)

arc_len = angle * radius

print(angle)
print(arc_len)


