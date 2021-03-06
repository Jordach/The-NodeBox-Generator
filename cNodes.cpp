#include "cNodes.h"

cNode::cNode(IrrlichtDevice* mdevice, ed_data* n_ed){
	id=0;
	device = mdevice;
	driver = device->getVideoDriver();
	smgr = device->getSceneManager();
	editor=n_ed;
	number=0;
	menu=NULL;

	for (int i=0;i<NODEB_MAX;i++){
		boxes[i]=NULL;
	}

	for (int i=0;i<17;i++){
		snappers[i]=NULL;
	}

	printf("Making 16 pixel snap grid: \n");
	for (int a=-8;a<9;a++){
		snappers[a+8] = a*0.0625;
		printf(">> %f\n",snappers[a+8]);
	}
}

sBox* cNode::addNodeBox(){
	// Set up structure
	boxes[number]=new sBox();

	// Add node
	boxes[number]->model=smgr->addCubeSceneNode(1,0,-1,vector3df(0,0,0));
	boxes[number]->model->setMaterialTexture(0, driver->getTexture("texture_box.png"));
	boxes[number]->model->setMaterialFlag(video::EMF_BILINEAR_FILTER, false);

	// Name it
	core::stringw nb=L"NodeBox ";
	nb+=(number+1);
	boxes[number]->model->setName(nb);
	boxes[number]->size=vector3df(1,1,1);
	boxes[number]->position=vector3df(0,0,0);

	// Switch the selected nodebox
	changeID(number);

	// Increment and print message
	number++;
	printf("Nodebox added\n");

	defrag();

	return boxes[number-1];
}

void cNode::switchFocus(ISceneNode* hit){
	for (int a=0;a<number;a++){
		if (boxes[a] && boxes[a]->model)
			if (boxes[a]->model==hit)
				changeID(a);
				return;

	}
	return;
}

void cNode::changeID(int n_id){
	id=n_id;

	updateTexts();
}

void cNode::update(){
	updateTexts();
}

void cNode::updateTexts(){
	if (getCurrentNodeBox() && number>id){
		core::stringw nb=L"NodeBox: ";
		nb+=boxes[id]->model->getName();
		editor->d_nb->setText(nb.c_str());

		core::stringw ps=L"Position: ";
		ps+=boxes[id]->model->getPosition().X;

		ps+=" , ";
		ps+=boxes[id]->model->getPosition().Y;
		ps+=" , ";
		ps+=boxes[id]->model->getPosition().Z;
		editor->d_pos->setText(ps.c_str());

		irr::core::vector3df extent = boxes[id]->size;

		core::stringw rt=L"Size: ";
		rt+=extent.X;
		rt+=" , ";
		rt+=extent.Y;
		rt+=" , ";
		rt+=extent.Z;

		editor->d_rot->setText(rt.c_str());
	}else{
		editor->d_nb->setText(L"NodeBox: -");
		editor->d_pos->setText(L"Position: - , - , -");
		editor->d_rot->setText(L"Size: - , - , -");
	}
}

void cNode::resize(int side,f32 dir){
	if (boxes[id] && number > id){
		switch (side)
		{
		case 0: //X
			setsizeObject(boxes[id], boxes[id]->size.X+dir, boxes[id]->size.Y, boxes[id]->size.Z);
			break;
		case 1: //Y
			setsizeObject(boxes[id], boxes[id]->size.X, boxes[id]->size.Y+dir, boxes[id]->size.Z);
			break;
		case 2: //Z
			setsizeObject(boxes[id], boxes[id]->size.X, boxes[id]->size.Y, boxes[id]->size.Z+dir);
			break;
		}
	}else{
		printf("[ERROR] No nodebox selected!\n");
	}
	updateTexts();
}

bool cNode::setsizeObject(sBox* input,f32 px,f32 py,f32 pz){
	// Check limits
	if (px < NODE_THIN|| py <  NODE_THIN || pz < NODE_THIN){
		return false;
	}

	// Hold the name temporary
	core::stringw nb=input->model->getName();

	// Remove the node
	input->model->remove();
	input->model=NULL;

	// Read the node
	input->model=smgr->addCubeSceneNode(1,0,-1,input->position);
	input->model->setMaterialTexture(0, driver->getTexture("texture_box.png"));
	input->model->setMaterialFlag(video::EMF_BILINEAR_FILTER, false);	
	input->model->setName(nb);
 
	// Get scale factor
	f32 sx = px/1;
	f32 sy = py/1;
	f32 sz = pz/1;

	// Print stuff
	printf("Resizing to (%f,",px);
	printf(" %f,",py);
	printf(" %f)\n",pz);

	// Do resize
	input->size=vector3df(px,py,pz);
	input->model->setScale(core::vector3df(sx, sy, sz));

	return true;
}

sBox* cNode::getCurrentNodeBox(){
	if (id==-1)
		return NULL;

	return boxes[id];
}

void cNode::resizeNodeBoxFace(sBox* nodebox,CDR_TYPE face, vector3df target){
	// Print stuff
	printf("Ray hit (%f,", target.X);
	printf(" %f,",target.Y);
	printf(" %f)\n",target.Z);

	f32 opp,change;
	vector3df tsize;

	// Do snapping
	if (editor->snapping==true){
		for (int i=0;i<15;i++){
			if (target.X > snappers[i]-0.0313 && target.X < snappers[i]+0.0313){
				target.X = snappers[i];
			}
			if (target.Y > snappers[i]-0.0313 && target.Y < snappers[i]+0.0313){
				target.Y = snappers[i];
			}
			if (target.Z > snappers[i]-0.0313 && target.Z < snappers[i]+0.0313){
				target.Z = snappers[i];
			}
		}
	}

	// Do node limiting
	if (editor->limiting==true){
		// X Axis
		if (target.X < -0.5)
			target.X = -0.5;
		else if (target.X > 0.5)
			target.X = 0.5;

		// Y Axis
		if (target.Y < -0.5)
			target.Y = -0.5;
		else if (target.Y > 0.5)
			target.Y = 0.5;

		// Z Axis
		if (target.Z < -0.5)
			target.Z = -0.5;
		else if (target.Z > 0.5)
			target.Z = 0.5;
	}

	switch (face){
	case CDR_X_P:
		

		// Resize
		tsize=nodebox->size; // Save the size
		opp = nodebox->position.X + ((float)nodebox->size.X / (float)2); // Get the corner before		
		tsize.X -= opp - target.X; // Adjust the size to now
		if (tsize.X <= 0) // Validate
			tsize.X=NODE_THIN;
		setsizeObject(nodebox,tsize.X,tsize.Y,tsize.Z); // Resize	

		// Change position
		change = (nodebox->position.X + ((float)nodebox->size.X / (float)2))-opp; 
		nodebox->position.X += change;
		break;
	case CDR_X_N:
		if (editor->limiting==true){
			if (target.X < -0.5)
				target.X = -0.5;
			else if (target.X > 0.5)
				target.X = 0.5;
		}

		// Resize
		tsize=nodebox->size; // Save the size		
		opp = nodebox->position.X - ((float)nodebox->size.X / (float)2); // Get the corner before			
		tsize.X += opp - target.X; // Adjust the size to now
		if (tsize.X <= 0) // Validate
			tsize.X=NODE_THIN;
		setsizeObject(nodebox,tsize.X,tsize.Y,tsize.Z); // Resize	

		// Change position
		change = (nodebox->position.X - ((float)nodebox->size.X / (float)2))-opp; 
		nodebox->position.X += change;
		break;
	case CDR_Y_P:
		if (editor->limiting==true){
			if (target.Y < -0.5)
				target.Y = -0.5;
			else if (target.Y > 0.5)
				target.Y = 0.5;
		}

		// Resize
		tsize=nodebox->size; // Save the size		
		opp = nodebox->position.Y + ((float)nodebox->size.Y / (float)2); // Get the corner before		
		tsize.Y -= opp - target.Y; // Adjust the size to now
		if (tsize.Y <= 0) // Validate
			tsize.Y=NODE_THIN;
		setsizeObject(nodebox,tsize.X,tsize.Y,tsize.Z); // Resize	

		// Change position
		change = (nodebox->position.Y + ((float)nodebox->size.Y / (float)2))-opp; 
		nodebox->position.Y += change;
		break;
	case CDR_Y_N:
		if (editor->snapping==true){
			if (target.Y > -0.05 && target.Y < 0.05)
				target.Y = 0;
			else if (target.Y > 0.20 && target.Y < 0.30)
				target.Y = 0.25;
			else if (target.Y > -0.30 && target.Y < -0.20)
				target.Y = -0.25;
		}
		if (editor->limiting==true){
			if (target.Y < -0.5)
				target.Y = -0.5;
			else if (target.Y > 0.5)
				target.Y = 0.5;
		}

		// Resize
		tsize=nodebox->size; // Save the size		
		opp = nodebox->position.Y - ((float)nodebox->size.Y / (float)2); // Get the corner before			
		tsize.Y += opp - target.Y; // Adjust the size to now
		if (tsize.Y <= 0) // Validate
			tsize.Y=NODE_THIN;
		setsizeObject(nodebox,tsize.X,tsize.Y,tsize.Z); // Resize	

		// Change position
		change = (nodebox->position.Y - ((float)nodebox->size.Y / (float)2))-opp; 
		nodebox->position.Y += change;
		break;
	case CDR_Z_P:
		if (editor->snapping==true){
			if (target.Z > -0.05 && target.Z < 0.05)
				target.Z = 0;
			else if (target.Z > 0.20 && target.Z < 0.30)
				target.Z = 0.25;
			else if (target.Z > -0.30 && target.Z < -0.20)
				target.Z = -0.25;
		}
		if (editor->limiting==true){
			if (target.Z < -0.5)
				target.Z = -0.5;
			else if (target.Z > 0.5)
				target.Z = 0.5;
		}

		// Resize
		tsize=nodebox->size; // Save the size		
		opp = nodebox->position.Z + ((float)nodebox->size.Z / (float)2); // Get the corner before		
		tsize.Z -= opp - target.Z; // Adjust the size to now
		if (tsize.Z <= 0) // Validate
			tsize.Z=NODE_THIN;
		setsizeObject(nodebox,tsize.X,tsize.Y,tsize.Z); // Resize	

		// Change position
		change = (nodebox->position.Z + ((float)nodebox->size.Z / (float)2))-opp; 
		nodebox->position.Z += change;
		break;
	case CDR_Z_N:
		if (editor->snapping==true){
			if (target.Z > -0.05 && target.Z < 0.05)
				target.Z = 0;
			else if (target.Z > 0.20 && target.Z < 0.30)
				target.Z = 0.25;
			else if (target.Z > -0.30 && target.Z < -0.20)
				target.Z = -0.25;
		}
		if (editor->limiting==true){
			if (target.Z < -0.5)
				target.Z = -0.5;
			else if (target.Z > 0.5)
				target.Z = 0.5;
		}

		// Resize
		tsize=nodebox->size; // Save the size		
		opp = nodebox->position.Z - ((float)nodebox->size.Z / (float)2); // Get the corner before			
		tsize.Z += opp - target.Z; // Adjust the size to now
		if (tsize.Z <= 0) // Validate
			tsize.Z=NODE_THIN;
		setsizeObject(nodebox,tsize.X,tsize.Y,tsize.Z); // Resize	

		// Change position
		change = (nodebox->position.Z - ((float)nodebox->size.Z / (float)2))-opp; 
		nodebox->position.Z += change;
		break;
	}

	return;
}

// This build individual nodebox tables
stringc* cNode::build(sBox* nodebox){
	if (!nodebox)
		return NULL;

	stringc* res = new stringc();
	res->append("{");

	// Bounds one
	res->append(stringc(nodebox->position.X - ((float)nodebox->size.X / (float)2)));
	res->append(",");
	res->append(stringc(nodebox->position.Y - ((float)nodebox->size.Y / (float)2)));
	res->append(",");
	res->append(stringc(nodebox->position.Z - ((float)nodebox->size.Z / (float)2)));
	res->append(",");

	// Bounds two
	res->append(stringc(nodebox->position.X + ((float)nodebox->size.X / (float)2)));
	res->append(",");
	res->append(stringc(nodebox->position.Y + ((float)nodebox->size.Y / (float)2)));
	res->append(",");
	res->append(stringc(nodebox->position.Z + ((float)nodebox->size.Z / (float)2)));

	res->append("}");
	res->append(", --");
	res->append(nodebox->model->getName());

	return res;
}

// This puts all the nodeboxes together
stringc* cNode::build(BUILD_TYPE type){
	
	stringc* res = new stringc();

	if (type == NBT_FULL)
		res->append("minetest.register_node(\"test:node\",{\n");

	if (type >= NBT_NBS)
		if (type == NBT_FULL)
			res->append("\tdrawtype=\"nodebox\",\n\tparamtype = \"light\",\n\tnode_box = {\n\t\ttype = \"fixed\",\n\t\tfixed = {\n");
		else
			res->append("drawtype=\"nodebox\",\nparamtype = \"light\",\nnode_box = {\n\ttype = \"fixed\",\n\tfixed = {\n");

	int a;
	for (a=0;a<number;a++){
		stringc tmp = *build(boxes[a]);

		if (tmp!=NULL){
			for (int i=0;i<type;i++){
				res->append("\t");
				printf("tab");
			}
			res->append(tmp);
			res->append("\n");
		}
	}

	if (type >= NBT_NBS)
		if (type == NBT_FULL)
			res->append("\t\t}\n\t}\n");
		else
			res->append("\t}\n}\n");

	if (type == NBT_FULL)
		res->append("})");

	// Print result
	printf("\n===== res =====\n");
	printf("%s\n",res->c_str());
	printf("===============\n");
	return res;
}

void cNode::deleteNodebox(sBox* nodebox){
	if (!nodebox)
		return;

	for (int a=0;a<number;a++){
		if (boxes[a] && boxes[a]->model)
			if (boxes[a]==nodebox){
				delete boxes[a];
				boxes[a]=NULL;

				if (id==a)
					id=-1;

				defrag();
				return;
			}

	}
}

void cNode::defrag(){
	// Delete context menu stuff
	if (menu)
	{
		menu->removeAllItems();
		menu->addItem(L"Set texture...",-1,false);
		menu->addSeparator();
		menu->addItem(L"Add a nodebox",GUI_ID_BOX);
		menu->addItem(L"Nodebox properties",GUI_ID_TEXT,false);
		menu->addItem(L"Delete current nodebox",GUI_ID_DELETENB);
		menu->addSeparator();
	}

	int a=0;
	for (int i=0;i<NODEB_MAX;i++){
		if (boxes[i]!=NULL){
			boxes[a]=boxes[i];

			// Add menu item
			if (menu)
			{
				menu->addItem(convert(boxes[a]->model->getName()),230+a);
			}

			if (id==i)
				id=a;
			a++;
		}else{
			boxes[a]=NULL;
		}
	}

	number = a;

#ifdef _DEBUG
	for (int i=0;i<NODEB_MAX;i++){
		if (boxes[i]!=NULL && boxes[i]->model->getName()){
			printf("%i> ",i);
			printf("%s \n",boxes[i]->model->getName());
		}else
			printf("%i> NULL \n",i);
	}

	printf("There are %i boxes\n",a);
#endif
}
