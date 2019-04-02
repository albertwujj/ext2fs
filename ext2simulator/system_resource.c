//
//  system_resource.c
//  
//
//  Created by Albert Wu on 3/7/19.
//


MINODE *mialloc() // allocate a FREE minode for use
{
int i;
for (i=0; i<NMINODE; i++){
MIONDE *mp = &minode[i];
if (mp->refCount == 0){
mp->refCount = 1;
return mp;
}
}
printf(“FS panic: out of minodes\n”);
return 0;
}
int midalloc(MINODE *mip) // release a used minode
{
mip->refCount = 0;
}
