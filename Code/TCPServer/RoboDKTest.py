from robodk import robolink

RDK = robolink.Robolink()
cam_handle = RDK.Item("Camera 1")

def capture_image(image_path):
    return RDK.Cam2D_Snapshot(image_path, cam_handle, 'Color')

#C:/Users/Thor9/Downloads
success = capture_image("C:/Users/Thor9/Downloads/Test.jpg")
if success:
    print("Image captured")
else:
    print("Error")