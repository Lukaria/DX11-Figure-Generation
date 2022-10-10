This program is based on Microsoft DirectX 11 Tutorial 06. A possibility of figure generation was added by me. Originally this code is created as my lab work for DX11
## Figure Generation
Program generates figure based on icosahedron (cause it was the main aim). 

![](https://upload.wikimedia.org/wikipedia/commons/6/61/%D0%92%D0%BF%D0%B8%D1%81%D0%B0%D0%BD%D0%BD%D1%8B%D0%B9_%D0%BF%D1%80%D0%B0%D0%B2%D0%B8%D0%BB%D1%8C%D0%BD%D1%8B%D0%B9_%D0%B8%D0%BA%D0%BE%D1%81%D0%B0%D1%8D%D0%B4%D1%80.gif)
>Icosahedron

Edge size of figure and some other stuff is calculated by math formulas written in global scope. 
All you should do to generate another figure is to change number of middle points (10 by default for icosahedron). Number of middle points must be even number greater than 3.

![](https://github.com/Lukaria/DX11-Figure-Generation/blob/master/Images/Icosahedron.png)
>Middle points allocated between two red lines. Strictly speaking, they are all points except the bottom and the top one

Also you can change the radius of circumscribed circle, but it will affect only on the size of the figure.

If number of points are increased, the result of generation will tend to smth like this:

![](https://github.com/Lukaria/DX11-Figure-Generation/blob/master/Images/example.png)
>An example of points increasing 

The most significant part of generation is placed in *figure2()* function.
## How it works
### Points calculation
At first, algorithm calculate an array of points that will be used futher. Points coords are calculated as if they were a circle points. Points can be placed on the same imaginary circle, but their height must be different. Next gif can give a good understanding of how generation of middle points looks like on a circle:

![](https://github.com/Lukaria/DX11-Figure-Generation/blob/master/Images/circle.gif)
>Generation of middle points. Blue and red points have different height

Code:
```C++
for (int i = 0, j = 0; i < arraysize; i++) {
        if (i % 2 == 0) { //height calculation 
            Y = Length + Height / 2;
            Alpha = 0.0f;
        }
        else {
            Y = Length;
            Alpha = -XM_2PI / points;
            ++j;
        }

        X = (Side / 2) * cos(2 * XM_2PI / points * j + Alpha);//cos is for x coord
        Z = (Side / 2) * sin(2 * XM_2PI / points * j + Alpha);//sin is for y coord in Cartesian coordinates

        array[i] = { X, Y, Z };
    }
```
*Length* variable equals to height of pyramid (painted in blue on a very first image);

*Height* - equals to height between two pyramids;

*2 * XM_2PI / points * j* -  steering angle to place points correctly on a circle;

Finally, *Alpha* is nothing but an additional steering angle for points. As you can see, it affects only on odd numbers (red points on the last gif).

Then program set coords for the top and the bottom points that cant be included in loop:
```C++
	array[arraysize - 2] = { 0.0f, 2 * Length + Height / 2, 0.0f }; 
	array[arraysize - 1] = { 0.0f, 0.0f, 0.0f };
```
Its important to know that an array of points have two more points than real figure. This additional points have the same coordinates as the first two, so subsequent cycles can perform correctly at the end.

### Normals calculation
To calculate a normal to a plane we need only 3 points of this plane. So, if we have points with coords

A(x1, y1, z1);
B(x2, y2, z2);
C(x3, y3, z3);

According to a plane equation:

![](https://github.com/Lukaria/DX11-Figure-Generation/blob/master/Images/matrix.png)

So, the coefficients of *i, j, k* (matrix determinants) are the *x, y, z* of normal vector. Then, normal must be normalized, otherwise planes (polygons) will be illuminated too much and real color will be distored.

Code:

```C++
XMFLOAT3 getNormal(const XMFLOAT3 dot1, const XMFLOAT3 dot2, const XMFLOAT3 dot3) {
		float res_x, res_y, res_z;
		res_x = (dot2.y - dot1.y) * (dot3.z - dot1.z) - ((dot2.z - dot1.z) * (dot3.y - dot1.y)); 
		res_y = (dot2.x - dot1.x) * (dot3.z - dot1.z) - ((dot2.z - dot1.z) * (dot3.x - dot1.x)); 
		res_z = (dot2.x - dot1.x) * (dot3.y - dot1.y) - ((dot2.y - dot1.y) * (dot3.x - dot1.x)); 
		return getNormalized(XMFLOAT3{ res_x, -res_y, res_z }); 
}
```
Loops to calculate all normals that will be used:

```C++
	//middle normals
    for (int i = 0; i < normsize/2; i++) {
        if (i % 2 == 0) {
            normals[i] = getNormal(array[i], array[i + 2], array[i + 1]);
        }
        else {
            normals[i] = getNormal(array[i], array[i + 1], array[i + 2]);
        }
    }
	//upper normals
    for (int i = normsize/2, j = 0; i < 3 * normsize / 4; i++, j += 2) {
        normals[i] = getNormal(array[j], array[arraysize - 2], array[j + 2]);
    }
```
Different middle normals must be calculated in a different manner. The only difference is the order of points, because it affects a normal direction. Otherwise, planes will be illuminated when a light source well be on the contrary point of space.

![](https://github.com/Lukaria/DX11-Figure-Generation/blob/master/Images/planes.png)
>same plane and points, but the order is different

Upper normals also calculating a little different. The second point must be choosed as the top one of figure cannot be changed. Same loop for lower normals.

### Vertices determining
Its very easy to create vertices array, there are only a few rules: 
  1. Every point must be included the same number of times as the amount of its normals
  2. Every 3 points have the same normal (because they created a plane). 
  
Code:
```C++
const int vertsize = 6 * points; 
    SimpleVertex vertices2[vertsize];
    for (int i = 0, j = 0, k = 0; i < vertsize/2; i += 3, j += 1, k++) {
        vertices2[i] = { array[j], normals[k] };
        vertices2[i + 1] = { array[j + 1], normals[k] };
        vertices2[i + 2] = { array[j + 2], normals[k] };
    }
    for (int j = 0, k = normsize/2, i = vertsize / 2; i < 3 * vertsize / 4; i+=3, j+=2, k++) {
        vertices2[i] = { array[j], normals[k] };
        vertices2[i + 1] = { array[arraysize - 2], normals[k] }//top point
        vertices2[i + 2] = { array[j + 2], normals[k] };
    }
```

### Indices determining
An array of indices must be filled correctly to render. Thats why some points goes clockwise and others - counterclockwise.

Code:
```C++
    const int indsize = points * 6; //same as number of vertices
    WORD indices2[indsize];
    for (int i = 0, j = 0; i < indsize/2; i += 3, j++) {
        if (j % 2 == 0) { //clockwise
            indices2[i] = i;
            indices2[i + 1] = i + 2;
            indices2[i + 2] = i + 1;  
        }
        else { //also should be clockwise to render correctly
            indices2[i] = i;
            indices2[i + 1] = i + 1;
            indices2[i + 2] = i + 2;
        }
    }
    //upper indices. 1/4 of all
    for (int i = indsize/2, j = 0; j < points/2; i += 3, j++) {
        indices2[i] = i;
        indices2[i + 1] = i + 1;
        indices2[i + 2] = i + 2;
    } //same for lower indices
```

## Results
The result for 10 middle points:

![](https://github.com/Lukaria/DX11-Figure-Generation/blob/master/Images/results.png)





