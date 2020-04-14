Getting STM32 CAD files, footprints and schematic objects, into EagleCAD.

- Navigate to https://www.st.com/
- Search the Resources section for the chip series, followed by "OrCad", like "STM32F3 orcad".
- Download the OrCad Symbol and Footprint files and unzip the file.
![1](../images/stm32 orcad files to eaglecad/1.jpeg)


- Navigate to https://www.ultralibrarian.com/products/online-reader and go to 'Convert BXLs Now'.

![2](../images/stm32 orcad files to eaglecad/2.jpeg)

- Upload the desired chip model BXL file.
- Go straight to Download Now.

![3](../images/stm32 orcad files to eaglecad/3.jpeg)

- In the CAD Format selection, find Eagle and check the box for v6+, and download.

![4](../images/stm32 orcad files to eaglecad/4.jpeg)

- You now have an EagleCAD .scr script file.

![5](../images/stm32 orcad files to eaglecad/5.jpeg)

- Open Eagle and go to File > New > Library.

![6](../images/stm32 orcad files to eaglecad/6.jpeg)

- At this window now go to File > Execute Script...

![7](../images/stm32 orcad files to eaglecad/7.jpeg)

- Once the script is opened you should see appear the schematic, footprint and device in the library. The library can now be saved.

![8](../images/stm32 orcad files to eaglecad/8.jpeg)
