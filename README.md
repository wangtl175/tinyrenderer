# tinyrenderer
An implement of https://github.com/ssloy/tinyrenderer

# Lession 4: Perspective Projection
> 透视投影是为了获得接近真实三维物体的视觉效果而在二维的纸或者画布平面上绘图或者彩现的一种方法，它也称为透视图。透视投影的绘制必须根据已有的几何规则进行

## 二维几何
### Linear transformation 线性转换
对点(x, y)进行线性转换可以写成以下形式

![img.png](imags/img.png)

对一系列点做线性转换

![img.png](imags/img1.png)

### Affine transformation 仿射转换

线性转换加上平移

![img.png](imags/img2.png)

## Homogeneous coordinates 齐次坐标系
齐次坐标是将一个原本n维的向量用一个n+1维的向量来表示

使用齐次坐标系，可以将仿射转换用一个矩阵表示

矩阵的最后一行，和透视投影中的视角(camera)有关

## 三维几何
假设camera在(0,0,c)上， 计算一个点(x,y,z)的投影

![img.png](imags/img3.png)

计算出来的点，可以用于渲染，用上z-buffer使用计算出来的z。之前渲染出来的画像，相当于c趋于无穷大的时候。