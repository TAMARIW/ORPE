#!/usr/bin/env python
import cv2
from cv2 import waitKey
import numpy as np
import os
import glob
import time

# Defining the dimensions of checkerboard
CHECKERBOARD = (13, 24)
criteria = (cv2.TERM_CRITERIA_EPS + cv2.TERM_CRITERIA_MAX_ITER, 30, 0.001)

# Creating vector to store vectors of 3D points for each checkerboard image
objpoints = []
# Creating vector to store vectors of 2D points for each checkerboard image
imgpoints = []

# Defining the world coordinates for 3D points
objp = np.zeros((1, CHECKERBOARD[0] * CHECKERBOARD[1], 3), np.float32)
objp[0,:,:2] = np.mgrid[0:CHECKERBOARD[0], 0:CHECKERBOARD[1]].T.reshape(-1, 2)
prev_img_shape = None

vid = "C:/Users/Chris/Desktop/Bachelor_Arbeit/data/CamCalVid/CamCalVid.mp4"

imageNum = 0

cap = cv2.VideoCapture(vid)
while(cap.isOpened()):

    ret, frame = cap.read()

    if not ret:
        break
    else:

        cv2.imshow("Esc to finish. s to save. Other to continue", cv2.resize(frame, [1280, 720]))

        key = cv2.waitKey(0)

        if key == 27: # ESC was pressed
            break
        elif key == ord('s'):
            cv2.imwrite("C:/Users/Chris/Desktop/Bachelor_Arbeit/data/CamCalImages/CalImage" + str(imageNum) + ".png", frame)
            imageNum += 1



# Extracting path of individual image stored in a given directory
images = glob.glob('../data/CamCalImages/*.png')
for fname in images:
    img = cv2.imread(fname)
    img = cv2.resize(img, (1920, 1080))
    showimg = img.copy()
    cv2.imshow('img',cv2.resize(showimg, [1280, 720]))
    waitKey(500)
    gray = cv2.cvtColor(img,cv2.COLOR_BGR2GRAY)
    print("Searching for chessboard...")
    # Find the chess board corners
    # If desired number of corners are found in the image then ret = true
    ret, corners = cv2.findChessboardCorners(gray, CHECKERBOARD, cv2.CALIB_CB_ADAPTIVE_THRESH + cv2.CALIB_CB_FAST_CHECK + cv2.CALIB_CB_NORMALIZE_IMAGE)
    print("Found chessboard!")
    """
    If desired number of corner are detected,
    we refine the pixel coordinates and display
    them on the images of checker board
    """

    if ret == True:

        print("Output is true!")

        # refining pixel coordinates for given 2d points.
        corners2 = cv2.cornerSubPix(gray, corners, (11,11),(-1,-1), criteria)

        # Draw and display the corners
        img = cv2.drawChessboardCorners(img, CHECKERBOARD, corners2, ret)
    
    showimg = img.copy()
    cv2.imshow('Points',cv2.resize(showimg, [1280, 720]))

    key = cv2.waitKey(0)
    if key == ord('s'):
        objpoints.append(objp)
        imgpoints.append(corners2)

cv2.destroyAllWindows()

h,w = img.shape[:2]

"""
Performing camera calibration by
passing the value of known 3D points (objpoints)
and corresponding pixel coordinates of the
detected corners (imgpoints)
"""
ret, mtx, dist, rvecs, tvecs = cv2.calibrateCamera(objpoints, imgpoints, gray.shape[::-1], None, None)

print("Camera matrix : \n")
print(mtx)
print("dist : \n")
print(dist)
print("rvecs : \n")
print(rvecs)
print("tvecs : \n")
print(tvecs)
