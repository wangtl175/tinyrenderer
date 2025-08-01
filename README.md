# tinyrenderer

An implement of https://github.com/ssloy/tinyrenderer

中文参考 https://supercodepower.com/docs/toy-renderer/index

# Lession 1

[obj文件](https://en.wikipedia.org/wiki/Wavefront_.obj_file)

[中文](https://blog.csdn.net/shebao3333/article/details/132094257)

# Lession 2

## 重心坐标

$\triangle ABC$所在平面的点$P$，有

$$
\vec{AP} = u \vec{AB} + v \vec{AC} \\
P - A = u (B - A) + v (A -C)  \\
P =  (1-u-v)A + u B +vC
$$

称$(1-u-v, u, v)$是$P$关于$A, B,C$的重心坐标。如何求解$u,v$

$$
\vec{AP} = u \vec{AB} + v \vec{AC} \\
u\vec{AB} + v \vec{AC} + \vec{PA} = 0
$$

也就是说

$$
u\vec{AB}_x + v \vec{AC}_x + \vec{PA}_x = 0 \\
u\vec{AB}_y + v \vec{AC}_y + \vec{PA}_y = 0
$$

因此，$(u,v,1)$同时垂直于向量$(\vec{AB}_x,\vec{AC}_x,\vec{PA}_x)$和$(\vec{AB}_y,\vec{AC}_y,\vec{PA}_y)$，假设这两个向量的叉乘为$(e,f,g)$，且$g \neq0$，则$u=\frac{e}{g}, v=\frac{f}{g}$；如果$g=0$，$\triangle ABC$退化成一条直线了。

## Back-face culling

用于剔除那些处于后背的面，减少需要渲染的面。

### 向量叉乘的方向

$\vec{a} \times \vec{b}$的方向可以右手法则判断，四指先指向$\vec{a}$的方向，然后向$\vec{b}$弯曲，拇指即为$\vec{a} \times \vec{b}$的方向。

一个平面的法向量可以通过平面内两个非线性相关的向量的叉乘求出来。平面的法向量有无数个，单位法向量只有2个，分别指向两个不同的方向。

在obj文件里，三个点$V_0$、$V_1$、$V_2$确定一个平面，取$\vec{V_2V_0} \times \vec{V_1V_0}$的方向为法向量的方向。如果单位法向量与光源向量的点乘小于0，说明这个面是背光的，无需渲染，这就是back-face culling。

# Lession 4: Perspective Projection

> 投影就是把三维的场景变成观测到的二维图像，分为正交投影和透视投影
>
> 透视投影和人眼观测到的成像差不多，有近大远小的效果；正交投影则没有近大远小的变化

![img.png](imags/img7.png)

为什么会近大远小呢？如下图zy坐标系上，A和B的y坐标一样，但是在2c处形成的投影则是A''处于更高的地方

![img.png](imags/img8.png)

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

![img.png](imags/img6.png)

矩阵的最后一行，和透视投影中的视角(camera)有关

## 三维几何

假设camera在(0,0,c)上， 计算一个点(x,y,z)的投影

![img.png](imags/img3.png)

计算出来的点，可以用于渲染，用上z-buffer使用计算出来的z。之前渲染出来的画像，相当于c趋于无穷大的时候。

# Lesson 5

> Gouraud Shading https://blog.csdn.net/qq_36653924/article/details/130976716

## Change of basis in 3D space

这里介绍了在(i,j,k)坐标系下的坐标(x,y,z)，和(i',j',k')坐标系下的坐标(x',y',z')如何互相转换。先做平移，然后做线性转换，即一次仿射

![img.png](imags/img4.png)

> https://blog.csdn.net/weixin_44179561/article/details/124149297

现在有两个坐标系，一个是世界坐标系，另一个是摄像机坐标系，我们需要将世界坐标系中的坐标转换成摄像机坐标系内的坐标

## Let us create our own gluLookAt

这里给出中心点坐标c，摄像机坐标e，向量u（摄像机坐标系中，zy平面中的一个向量）。c是摄像机视角的正中心，以它为坐标系的原点，摄像机在z轴上

```c++
void lookat(Vec3f eye, Vec3f center, Vec3f up) {
    Vec3f z = (eye-center).normalize();
    Vec3f x = cross(up,z).normalize();
    Vec3f y = cross(z,x).normalize();
    Matrix Minv = Matrix::identity();
    Matrix Tr   = Matrix::identity();
    for (int i=0; i<3; i++) {
        Minv[0][i] = x[i];
        Minv[1][i] = y[i];
        Minv[2][i] = z[i];
        Tr[i][3] = -center[i];
    }
    ModelView = Minv*Tr;
}
```

这段代码，先计算出摄影机坐标系的xyz轴的基向量（用世界坐标系表示），也就是下图中的(i',j',k')。由于(i,j,k)是基向量，所以M其实是等于(i',j',k')的。

![img.png](imags/img5.png)

由于矩阵的特殊性质，M的逆是等于M的转置的，下面代码可以验证

```python
import numpy as np
def normal(v):
    return v / np.sqrt((v * v).sum())
v1 = np.array([1, 2, 3])
v2 = np.array([1, 1, 1])
z = normal(v1)
x = normal(np.cross(v2, z))
y = normal(np.cross(z, x))
M = np.mat([x, y, z])
print(M)
print(M.T)
```

这里有个小问题，就是上面是先做了平移，然后再做

## Transformation of normal vector

如果有一个模型以及它的法向量，并且这个模型用一个矩阵进行仿射(affine mapping)转换，那么法向量就是用这个矩阵的逆进行转换
