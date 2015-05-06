
import os;
import glob;
import string;

pixmaps = glob.glob("*.png");

pixmaps.sort();

#Generate include files

f=open("theme_data.h","wb")

f.write("// THIS FILE HAS BEEN AUTOGENERATED, DONT EDIT!!\n");

f.write("\n\n");

f.write("\n\n\n");
	

	
for x in pixmaps:
	
	var_str=x[:-4]+"_png";
	
	f.write("static const unsigned char "+ var_str +"[]={\n");
	
	pngf=open(x,"rb");
	
	b=pngf.read(1);
	while(len(b)==1):
		f.write(hex(ord(b)))
		b=pngf.read(1);
		if (len(b)==1):
			f.write(",")
			
	f.write("\n};\n\n\n");
	pngf.close();
	
f.close();

