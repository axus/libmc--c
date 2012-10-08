/*
  libmc--c main
  Test program for mc--c library

  Copyright 2010 axus

    libmc--c is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as
    published by the Free Software Foundation, either version 3 of the
    License, or at your option) any later version.

    libmc--c is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this program.  If not, see
    <http://www.gnu.org/licenses/>.
*/


//STL
#include <iostream>
using std::cout;
using std::hex;
using std::dec;
using std::endl;
using std::string;

//mc--
#include <mc--/Mobiles.hpp>
#include "UserInterface.hpp"
using mc__::World;
using mc__::Player;
using mc__::Events;
using mc__::Mobiles;
using mc__::Block;

using mc__::SLOT_EQ_MAX;
using mc__::player_inv_slots;

//SFML
#include <SFML/System/Clock.hpp>

//
// Functions
//

void debugKey(const World& world, int32_t X, int8_t Y, int32_t Z)
{
    cout << (int)X << "," << (short)Y << "," << (int)Z << "0x" << hex
    << world.getKey(X, Z) << endl << dec;
}

//
//Create game chunk(s) for test world
//
void genWorld(World& world)
{
    
    //Create a square mile of solid terrain
    int32_t gen_X, gen_Z;
    for (gen_X = -160; gen_X <= 160; gen_X += 16) {
    for (gen_Z = -160; gen_Z <= 160; gen_Z += 16) {
        //Generate 16x16 terrain at X,0,Z, height of 64
        if (!world.genFlatGrass(gen_X, 0, gen_Z, 64)) {
            cout << "Error: genFlatGrass ";
            debugKey(world, 0,0,16);
        }
    }}
    
    //Create empty void
    world.genClosedVoid(64, 64);

    //Draw all block IDs
    if (!world.genChunkTest(0, 64, 0)) {
        cout << "Error: genChunkTest ";
        debugKey(world, 0,64,0);
    }
    if (!world.genChunkTest(0, 65, 31)) {
        cout << "Error: genChunkTest ";
        debugKey(world, 0,65,31);
    }
    
    //Grow some trees
    world.genTree(11, 64, 5);
    world.genTree(4, 64, 7, 3, 10, 3);

    //Other tree types (metadata 1, 2)
    world.genTree(4, 64, 19, 5, 12, 5, 1);
    world.genTree(12, 64, 19, 5, 8, 5, 2);


    //Glass pillar
    if (!world.genWall(24, 64, 2, 2, 17, 2, 20)) {
        cout << "Error: genWall ";
        debugKey(world, 3, 64, 6);
    }
    //Brick pillar next to glass
    if (!world.genWall(26, 64, 2, 1, 17, 2, 45)) {
        cout << "Error: genWall ";
        debugKey(world, 3, 64, 6);
    }
    //Single wallsign embedded in brick wall
    Block sign1 = {68, 0, 0, 0}; //wallsign, no metadata
    mc__::Chunk *chunk1 = new mc__::Chunk(0, 0, 0, 26, 65, 3);//location
    chunk1->block_array[0] = sign1;
    
    //Add the chunk with the wallsign
    world.addMapChunk(chunk1);
    delete chunk1;

    //Draw dyed wool    
    Block dyed_wool = {35, 0, 0, 0}; //wool, 0 metadata
    chunk1 = new mc__::Chunk(15, 0, 0, -16, 65, 0);
    for (gen_X = 0; gen_X < 16; gen_X++) {
        chunk1->block_array[gen_X] = dyed_wool;
        dyed_wool.metadata++;
    }
    world.addMapChunk(chunk1);
    delete chunk1;

    
    //Table with cakes
    Block workbench1 = {mc__::Blk::Workbench, 0, 0, 0};//workbench, no metadata
    Block cake1 = { mc__::Blk::Cake, 0, 0, 0};
    chunk1 = new mc__::Chunk(10, 1, 2, 17, 64, 17);    //1x1x1, location
    for (gen_X = 0; gen_X <= 10; gen_X++) {
        for (gen_Z = 0; gen_Z <= 2; gen_Z++) {
            //Y=0 is table
            chunk1->block_array[ gen_X*3*2 + gen_Z*2] = workbench1;
            
            //Place cake at intervals on table
            if (gen_Z == 1 && (gen_X & 1) == 0) {
                chunk1->block_array[ gen_X*3*2 + gen_Z*2 + 1] = cake1;
                cake1.metadata += 1;    //Ate a little cake...
            }
        }
    }
    //Add the chunk with cake
    world.addMapChunk(chunk1);
    delete chunk1;
    
    //Beds
    chunk1 = new mc__::Chunk(3, 0, 3, 16, 64, 2);    //4x1x4
    //Point south
    Block bed1 = {mc__::Blk::Bed, 0x0, 0, 0};   //foot
    Block bed2 = {mc__::Blk::Bed, 0x8, 0, 0};   //head
    chunk1->block_array[1] = bed1;
    chunk1->block_array[2] = bed2;

    //Point west
    bed1.metadata = 0x8|0x1;
    bed2.metadata = 0x0|0x1;
    chunk1->block_array[4] = bed1;
    chunk1->block_array[8] = bed2;

    //Point east
    bed1.metadata = 0x8|0x3;
    bed2.metadata = 0x0|0x3;
    chunk1->block_array[7] = bed2;
    chunk1->block_array[11] = bed1;

    //Point north
    bed1.metadata = 0x8|0x2;   
    bed2.metadata = 0x0|0x2;
    chunk1->block_array[13] = bed1;
    chunk1->block_array[14] = bed2;

    //Add the chunk with beds
    world.addMapChunk(chunk1);
    delete chunk1;

    //Add crops with metadata 0-7
    chunk1 = new mc__::Chunk(7, 0, 0, 4, 64, 18);   //Row of 8 crops
    Block crops = {59, 0, 0, 0};
    for (gen_X = 0; gen_X < 8; gen_X++) {
        //Increase metadata for each crop
        crops.metadata = gen_X;
        chunk1->block_array[ gen_X ] = crops;
    }
    //Add the chunk with crops
    world.addMapChunk(chunk1);
    delete chunk1;
    
    //Melon stem test with metadata 0-7
    chunk1 = new mc__::Chunk(9, 0, 0, 4, 64, 17);   //Row of melon stems, melon
    crops.blockID = mc__::Blk::MelonStem;
    for (gen_X = 0; gen_X < 8; gen_X++) {
        //Increase metadata for each crop
        crops.metadata = gen_X;
        chunk1->block_array[ gen_X ] = crops;
    }
    //stem next to pumpkin
    chunk1->block_array[ gen_X ] = crops;
    crops.blockID = mc__::Blk::Melon;
    chunk1->block_array[ gen_X + 1] = crops;
    
    //Add the chunk with crops
    world.addMapChunk(chunk1);
    delete chunk1;

    //Pumpkin stem test with metadata 0-7
    chunk1 = new mc__::Chunk(9, 0, 0, 4, 64, 16);   //Row of melon stems, melon
    crops.blockID = mc__::Blk::PumpkinStem;
    for (gen_X = 0; gen_X < 8; gen_X++) {
        //Increase metadata for each crop
        crops.metadata = gen_X;
        chunk1->block_array[ gen_X ] = crops;
    }
    //stem next to pumpkin
    chunk1->block_array[ gen_X ] = crops;
    crops.blockID = mc__::Blk::Pumpkin;
    chunk1->block_array[ gen_X + 1] = crops;
    
    //Add the chunk with crops
    world.addMapChunk(chunk1);
    delete chunk1;


    //Stems and melons
    Block pumpkin =     {mc__::Blk::Pumpkin, 0, 0, 0};
    Block stemPumpkin = {mc__::Blk::PumpkinStem, 0x7, 0, 0};
    Block melon =       {mc__::Blk::Melon, 0, 0, 0};
    Block stemMelon =   {mc__::Blk::MelonStem, 0x7, 0, 0};
    chunk1 = new mc__::Chunk(3, 0, 3, 0, 64, 16);   //Row of melon stems, melon
    
    //set stems next to melons
    chunk1->block_array[ 0] = pumpkin;
    chunk1->block_array[ 1] = stemPumpkin;
    chunk1->block_array[ 2] = stemPumpkin;
    chunk1->block_array[ 3] = melon;
    
    chunk1->block_array[ 4] = melon;
    chunk1->block_array[ 5] = stemMelon;
    chunk1->block_array[ 6] = pumpkin;
    chunk1->block_array[ 7] = stemMelon;
    
    chunk1->block_array[ 8] = stemPumpkin;
    chunk1->block_array[ 9] = pumpkin;
    chunk1->block_array[10] = stemMelon;
    chunk1->block_array[11] = pumpkin;
    
    chunk1->block_array[12] = stemMelon;
    chunk1->block_array[13] = melon;
    chunk1->block_array[14] = melon;
    chunk1->block_array[15] = stemPumpkin;
    
    //Add the chunk with stems and melons
    world.addMapChunk(chunk1);
    delete chunk1;

    
    //
    //Facecube test: dispenser, furnace, litfurnace, pumpkin, jack-o-lantern
    //
    chunk1 = new mc__::Chunk(13, 0, 1, 1, 64, 21);   //5 groups of 4 face items

    //Dispenser
    Block faceCube = {23, 4, 0, 0};
    faceCube.metadata = 4; chunk1->block_array[0] = faceCube;
    faceCube.metadata = 3; chunk1->block_array[1] = faceCube;
    faceCube.metadata = 2; chunk1->block_array[2] = faceCube;
    faceCube.metadata = 5; chunk1->block_array[3] = faceCube;
    
    //Furnace
    faceCube.blockID = 61;
    faceCube.metadata = 4; chunk1->block_array[6] = faceCube;
    faceCube.metadata = 3; chunk1->block_array[7] = faceCube;
    faceCube.metadata = 2; chunk1->block_array[8] = faceCube;
    faceCube.metadata = 5; chunk1->block_array[9] = faceCube;
    
    //Lit furnace
    faceCube.blockID = 62;
    faceCube.metadata = 4; chunk1->block_array[12] = faceCube;
    faceCube.metadata = 3; chunk1->block_array[13] = faceCube;
    faceCube.metadata = 2; chunk1->block_array[14] = faceCube;
    faceCube.metadata = 5; chunk1->block_array[15] = faceCube;
    
    //Pumpkin
    faceCube.blockID = 86;
    faceCube.metadata = 3; chunk1->block_array[18] = faceCube;
    faceCube.metadata = 2; chunk1->block_array[19] = faceCube;
    faceCube.metadata = 0; chunk1->block_array[20] = faceCube;
    faceCube.metadata = 1; chunk1->block_array[21] = faceCube;
    
    //Jack-o-Lantern
    faceCube.blockID = mc__::Blk::PumpkinOn;
    faceCube.metadata = 3; chunk1->block_array[24] = faceCube;
    faceCube.metadata = 2; chunk1->block_array[25] = faceCube;
    faceCube.metadata = 0; chunk1->block_array[26] = faceCube;
    faceCube.metadata = 1; chunk1->block_array[27] = faceCube;

    world.addMapChunk(chunk1);    
    delete chunk1;
    
    //
    //  Redstone wire test
    //
    chunk1 = new mc__::Chunk(5, 0, 2, -8, 64, 21);   //3x6 wire chunk

    //Wire
    Block wireBlock = {55, 0, 0, 0};
    Block wireTorch = {76, 5, 0, 0};
    //Square of 9 wires
    for (size_t i = 0; i < 9; i++) {
        wireBlock.metadata = i; chunk1->block_array[i] = wireBlock;
    }
    //3 wires around a redstone torch
    wireBlock.metadata = 12; chunk1->block_array[12] = wireBlock;
    wireTorch.metadata = 5;  chunk1->block_array[13] = wireTorch;
    wireBlock.metadata = 14; chunk1->block_array[14] = wireBlock;
    wireBlock.metadata = 15; chunk1->block_array[16] = wireBlock;
    //Add the chunk
    world.addMapChunk(chunk1);    
    delete chunk1;

    //
    // Chest test
    //
    chunk1 = new mc__::Chunk( 4, 0, 2, -13, 64, 20);   //3x5 area for chests
    Block chestBlock = { 54, 0, 0, 0};
    //single
    chunk1->block_array[0] = chestBlock;
    //2x1 on X
    chunk1->block_array[2] = chestBlock;
    chunk1->block_array[5] = chestBlock;
    //2x1 on Z
    chunk1->block_array[6] = chestBlock;
    chunk1->block_array[7] = chestBlock;
    //3x1 on Z, should look weird :)
    chunk1->block_array[12] = chestBlock;
    chunk1->block_array[13] = chestBlock;
    chunk1->block_array[14] = chestBlock;
    //Add the chunk
    world.addMapChunk(chunk1);    
    delete chunk1;

    //
    // Diode test
    //
    chunk1 = new mc__::Chunk( 7, 0, 3, -24, 64, 20);   //8x4 area for diodes
    Block diodeBlock = { 93, 0, 0, 0};
    chunk1->block_array[0] = diodeBlock; diodeBlock.metadata++;
    chunk1->block_array[2] = diodeBlock; diodeBlock.metadata++;
    chunk1->block_array[5] = diodeBlock; diodeBlock.metadata++;
    chunk1->block_array[7] = diodeBlock; diodeBlock.metadata++;
    chunk1->block_array[8] = diodeBlock; diodeBlock.metadata++;
    chunk1->block_array[10] = diodeBlock; diodeBlock.metadata++;
    chunk1->block_array[13] = diodeBlock; diodeBlock.metadata++;
    chunk1->block_array[15] = diodeBlock; diodeBlock.metadata++;
    chunk1->block_array[16] = diodeBlock; diodeBlock.metadata++;
    chunk1->block_array[18] = diodeBlock; diodeBlock.metadata++;
    chunk1->block_array[21] = diodeBlock; diodeBlock.metadata++;
    chunk1->block_array[23] = diodeBlock; diodeBlock.metadata++;
    chunk1->block_array[24] = diodeBlock; diodeBlock.metadata++;
    chunk1->block_array[26] = diodeBlock; diodeBlock.metadata++;
    chunk1->block_array[29] = diodeBlock; diodeBlock.metadata++;
    chunk1->block_array[31] = diodeBlock;
    //Add the chunk
    world.addMapChunk(chunk1);    
    delete chunk1;

    //
    // Track test
    //
    chunk1 = new mc__::Chunk( 3, 0, 3, -31, 64, 20);   //4x4 area for tracks
    Block glassBlock = { 20, 0, 0, 0};
    Block trackBlock = { mc__::Blk::Track, 0, 0, 0};
    
    trackBlock.metadata = 3; chunk1->block_array[0] = trackBlock;   //up N
    trackBlock.metadata = 6; chunk1->block_array[2] = trackBlock;   //turn NE
    trackBlock.metadata = 9; chunk1->block_array[3] = trackBlock;   //turn NW
    trackBlock.metadata = 1; chunk1->block_array[4] = trackBlock;   //N/S
    trackBlock.metadata = 7; chunk1->block_array[6] = trackBlock;   //turn SE
    trackBlock.metadata = 8; chunk1->block_array[7] = trackBlock;   //turn SW
    trackBlock.metadata = 2; chunk1->block_array[8] = trackBlock;   //up S
                             chunk1->block_array[12] = glassBlock;
    trackBlock.metadata = 4; chunk1->block_array[13] = trackBlock;  //up E
    trackBlock.metadata = 0; chunk1->block_array[14] = trackBlock;   //E/W
    trackBlock.metadata = 5; chunk1->block_array[15] = trackBlock;   //up W
                    
    //Add the chunk
    world.addMapChunk(chunk1);    
    delete chunk1;


    //
    //  Raised redstone, floorplate, and ladder test
    //
    chunk1 = new mc__::Chunk(3, 1, 3, -8, 65, 9);   //4x2x4 chunk
    Block woodPlate = { 72, 0, 0, 0};
    Block stonePlate = { 70, 0, 0, 0};
    Block ladderBlock = { 65, 2, 0, 0};
    wireBlock.metadata = 0; 
    //Surround redstone with glass
    chunk1->block_array[0] = ladderBlock; ladderBlock.metadata = 4;
    chunk1->block_array[2] = glassBlock;
    chunk1->block_array[4] = glassBlock;
    chunk1->block_array[6] = ladderBlock; ladderBlock.metadata = 5;
    chunk1->block_array[8] = glassBlock;
    chunk1->block_array[10] = wireBlock;
    chunk1->block_array[12] = wireBlock;
    chunk1->block_array[14] = glassBlock;
    chunk1->block_array[16] = glassBlock;
    chunk1->block_array[18] = wireBlock;
    chunk1->block_array[20] = wireBlock;
    chunk1->block_array[22] = glassBlock;
    chunk1->block_array[24] = ladderBlock; ladderBlock.metadata = 3;
    chunk1->block_array[26] = glassBlock;
    chunk1->block_array[28] = glassBlock;
    chunk1->block_array[30] = ladderBlock;
    
    //Connect to plates on row above
    chunk1->block_array[3] = stonePlate; stonePlate.metadata = 1;
    chunk1->block_array[15] = woodPlate; woodPlate.metadata = 1;
    chunk1->block_array[17] = woodPlate;
    chunk1->block_array[29] = stonePlate;
    //Add the chunk
    world.addMapChunk(chunk1);
    delete chunk1;

    //
    // Door test
    //
    chunk1 = new mc__::Chunk( 2, 1, 2, 0, 64, 7);   //3x2x3 area for doors
    Block doorBlock = { 64, 0, 0, 0}; //wood door
    doorBlock.metadata = 6 ; chunk1->block_array[0] = doorBlock;
    doorBlock.metadata += 8; chunk1->block_array[1] = doorBlock;
    doorBlock.metadata = 2 ; chunk1->block_array[2] = doorBlock;
    doorBlock.metadata += 8; chunk1->block_array[3] = doorBlock;
    doorBlock.metadata = 5 ; chunk1->block_array[4] = doorBlock;
    doorBlock.metadata += 8; chunk1->block_array[5] = doorBlock;
    doorBlock.metadata = 3 ; chunk1->block_array[6] = doorBlock;
    doorBlock.metadata += 8; chunk1->block_array[7] = doorBlock;
    doorBlock.metadata = 1 ; chunk1->block_array[10] = doorBlock;
    doorBlock.metadata += 8; chunk1->block_array[11] = doorBlock;
    doorBlock.metadata = 7 ; chunk1->block_array[12] = doorBlock;
    doorBlock.metadata += 8; chunk1->block_array[13] = doorBlock;
    doorBlock.metadata = 0 ; chunk1->block_array[14] = doorBlock;
    doorBlock.metadata += 8; chunk1->block_array[15] = doorBlock;
    doorBlock.metadata = 4 ; chunk1->block_array[16] = doorBlock;
    doorBlock.metadata += 8; chunk1->block_array[17] = doorBlock;
    //Add the chunk
    world.addMapChunk(chunk1);
    delete chunk1;

    chunk1 = new mc__::Chunk( 2, 1, 2, -4, 64, 4);   //3x2x3 area for doors
    doorBlock = { 71, 0, 0, 0}; //iron door
    doorBlock.metadata = 6 ; chunk1->block_array[0] = doorBlock;
    doorBlock.metadata += 8; chunk1->block_array[1] = doorBlock;
    doorBlock.metadata = 2 ; chunk1->block_array[2] = doorBlock;
    doorBlock.metadata += 8; chunk1->block_array[3] = doorBlock;
    doorBlock.metadata = 5 ; chunk1->block_array[4] = doorBlock;
    doorBlock.metadata += 8; chunk1->block_array[5] = doorBlock;
    doorBlock.metadata = 3 ; chunk1->block_array[6] = doorBlock;
    doorBlock.metadata += 8; chunk1->block_array[7] = doorBlock;
    doorBlock.metadata = 1 ; chunk1->block_array[10] = doorBlock;
    doorBlock.metadata += 8; chunk1->block_array[11] = doorBlock;
    doorBlock.metadata = 7 ; chunk1->block_array[12] = doorBlock;
    doorBlock.metadata += 8; chunk1->block_array[13] = doorBlock;
    doorBlock.metadata = 0 ; chunk1->block_array[14] = doorBlock;
    doorBlock.metadata += 8; chunk1->block_array[15] = doorBlock;
    doorBlock.metadata = 4 ; chunk1->block_array[16] = doorBlock;
    doorBlock.metadata += 8; chunk1->block_array[17] = doorBlock;
    //Add the chunk
    world.addMapChunk(chunk1);
    delete chunk1;

    //Stairs test
    chunk1 = new mc__::Chunk( 4, 0, 1, -9, 64, 3);   //5x1x2 area for stairs
    Block stairsBlock = { 53, 0, 0, 0};   //wood stairs
    stairsBlock.metadata = 0; chunk1->block_array[0] = stairsBlock;
    stairsBlock.metadata = 3; chunk1->block_array[1] = stairsBlock;
    stairsBlock.metadata = 2; chunk1->block_array[2] = stairsBlock;
    stairsBlock.metadata = 1; chunk1->block_array[3] = stairsBlock;
    stairsBlock = { 67, 0, 0, 0};   //cobble stairs
    stairsBlock.metadata = 0; chunk1->block_array[6] = stairsBlock;
    stairsBlock.metadata = 3; chunk1->block_array[7] = stairsBlock;
    stairsBlock.metadata = 2; chunk1->block_array[8] = stairsBlock;
    stairsBlock.metadata = 1; chunk1->block_array[9] = stairsBlock;
    //Add the chunk
    world.addMapChunk(chunk1);
    delete chunk1;

    //Fence and sign test
    chunk1 = new mc__::Chunk( 2, 1, 2, -14, 64, 3);   //3x2x3 area
    Block fenceBlock = { 85, 0, 0, 0};
    chunk1->block_array[0] = fenceBlock;
    chunk1->block_array[2] = fenceBlock;
    chunk1->block_array[4] = fenceBlock;
    chunk1->block_array[6] = fenceBlock;
    chunk1->block_array[8] = fenceBlock;
    chunk1->block_array[9] = fenceBlock;
    chunk1->block_array[10] = fenceBlock;
    chunk1->block_array[12] = fenceBlock;
    chunk1->block_array[14] = fenceBlock;
    chunk1->block_array[16] = fenceBlock;
    sign1.metadata = 4; chunk1->block_array[3] = sign1;
    sign1.metadata = 2; chunk1->block_array[7] = sign1;
    sign1.metadata = 3; chunk1->block_array[11] = sign1;
    sign1.metadata = 5; chunk1->block_array[15] = sign1;
    //Add the chunk
    world.addMapChunk(chunk1);
    delete chunk1;

    //Torch test
    chunk1 = new mc__::Chunk( 2, 0, 2, -13, 65, 9);   //3x1x3 area
    Block torchBlock = { 50, 5, 0, 0};
    torchBlock.metadata = 2; chunk1->block_array[1] = torchBlock;
    torchBlock.metadata = 4; chunk1->block_array[3] = torchBlock;
    chunk1->block_array[4] = glassBlock;
    torchBlock.metadata = 3; chunk1->block_array[5] = torchBlock;
    torchBlock.metadata = 1; chunk1->block_array[7] = torchBlock;
    //Add the chunk
    world.addMapChunk(chunk1);
    delete chunk1;

    //Signpost test (below torch test)
    chunk1 = new mc__::Chunk( 4, 0, 4, -14, 64, 8);   //5x1x5 area
    Block postBlock = { 63, 0, 0, 0};
    
    postBlock.metadata=0x6; chunk1->block_array[0] = postBlock;
    postBlock.metadata=0x5; chunk1->block_array[1] = postBlock;
    postBlock.metadata=0x4; chunk1->block_array[2] = postBlock;
    postBlock.metadata=0x3; chunk1->block_array[3] = postBlock;
    postBlock.metadata=0x2; chunk1->block_array[4] = postBlock;
    postBlock.metadata=0x7; chunk1->block_array[5] = postBlock;
    postBlock.metadata=0x1; chunk1->block_array[9] = postBlock;
    postBlock.metadata=0x8; chunk1->block_array[10] = postBlock;
    torchBlock.metadata=0x5; chunk1->block_array[12] = torchBlock;
    postBlock.metadata=0x0; chunk1->block_array[14] = postBlock;
    postBlock.metadata=0x9; chunk1->block_array[15] = postBlock;
    postBlock.metadata=0xF; chunk1->block_array[19] = postBlock;
    postBlock.metadata=0xA; chunk1->block_array[20] = postBlock;
    postBlock.metadata=0xB; chunk1->block_array[21] = postBlock;
    postBlock.metadata=0xC; chunk1->block_array[22] = postBlock;
    postBlock.metadata=0xD; chunk1->block_array[23] = postBlock;
    postBlock.metadata=0xE; chunk1->block_array[24] = postBlock;
    //Add the chunk
    world.addMapChunk(chunk1);
    delete chunk1;

    //Slab test
    chunk1 = new mc__::Chunk( 3, 0, 5, -21, 64, 3);   //4x1x6 area for slabs
    Block slab = { 44, 0, 0, 0};
    Block slabX2 = { 43, 0, 0, 0};
    Block other = { 1, 0, 0, 0};
    //Stone
    chunk1->block_array[0] = slab; slab.metadata++;
    chunk1->block_array[2] = slabX2; slabX2.metadata++;
    chunk1->block_array[4] = other; other.blockID = 24; 
    //Sandstone
    chunk1->block_array[7] = slab; slab.metadata++;
    chunk1->block_array[9] = slabX2; slabX2.metadata++;
    chunk1->block_array[11] = other; other.blockID = 5;
    //Wood
    chunk1->block_array[12] = slab; slab.metadata++;
    chunk1->block_array[14] = slabX2; slabX2.metadata++;
    chunk1->block_array[16] = other; other.blockID = 4;
    //Cobble
    chunk1->block_array[19] = slab; slab.metadata++;
    chunk1->block_array[21] = slabX2; slabX2.metadata++;
    chunk1->block_array[23] = other;
    //Add the chunk
    world.addMapChunk(chunk1);
    delete chunk1;

    // Glass pane test
    chunk1 = new mc__::Chunk( 4, 0, 4, -21, 64, 10); //5x1x5 area for pane test
    Block pane = { mc__::Blk::GlassPane, 0, 0, 0};  //Glass pane block, no meta
    
    //remember, when Y=1, Z=5 then index = z + x*5
    chunk1->block_array[0] = pane;
    chunk1->block_array[5] = pane;
    chunk1->block_array[10] = pane;

    chunk1->block_array[20] = pane;

    chunk1->block_array[22] = pane;
    chunk1->block_array[23] = pane;
    chunk1->block_array[24] = pane;

    chunk1->block_array[2] = pane;
    chunk1->block_array[3] = pane;
    chunk1->block_array[4] = pane;
    chunk1->block_array[7] = pane;
    chunk1->block_array[8] = pane;
    chunk1->block_array[9] = pane;
    chunk1->block_array[12] = pane;
    chunk1->block_array[13] = pane;
    chunk1->block_array[14] = pane;
    //Add the chunk
    world.addMapChunk(chunk1);
    delete chunk1;

    // Iron bar test
    chunk1 = new mc__::Chunk( 4, 0, 4, -21, 68, 10); //5x1x5 area for pane test
    Block bars = { mc__::Blk::IronBars, 0, 0, 0};  //Glass pane block, no meta
    
    //remember, when Y=1, Z=5 then index = z + x*5
    chunk1->block_array[0] = bars;
    chunk1->block_array[5] = bars;
    chunk1->block_array[10] = bars;

    chunk1->block_array[20] = bars;

    chunk1->block_array[22] = bars;
    chunk1->block_array[23] = bars;
    chunk1->block_array[24] = bars;

    chunk1->block_array[2] = bars;
    chunk1->block_array[3] = bars;
    chunk1->block_array[4] = bars;
    chunk1->block_array[7] = bars;
    chunk1->block_array[8] = bars;
    chunk1->block_array[9] = bars;
    chunk1->block_array[12] = bars;
    chunk1->block_array[13] = bars;
    chunk1->block_array[14] = bars;
    //Add the chunk
    world.addMapChunk(chunk1);
    delete chunk1;


    //Cactus test
    chunk1 = new mc__::Chunk( 0, 0, 15, 1, 64, -16); //1x1x15 area for cactus
    Block cactusBlock = { 81, 0, 0, 0};
    for (gen_Z=0; gen_Z < 16; gen_Z++) {
        cactusBlock.metadata = gen_Z;
        chunk1->block_array[gen_Z] = cactusBlock;
    }
    //Add the chunk
    world.addMapChunk(chunk1);
    delete chunk1;

    //
    //Move the world in these directions at start
    //
    world.spawn_X = -12;
    world.spawn_Y = 64;
    world.spawn_Z = 16;

    //Mark every chunk as updated
    world.redraw();
}

//Add some test objects to mobile items/entities
void addItems( mc__::Mobiles& mobiles, const mc__::World& world) {

    //Add all the block items
    for (uint16_t i = 0; i < 93; i++) {
        mobiles.addItem( mobiles.newEID(), i, 1,
            (world.spawn_X + i ) << 5,
            (world.spawn_Y ) << 5,
            (world.spawn_Z - 3) << 5, 0x2B);
    }
    
    //Add ALL the pickup items
    for (uint16_t i = 256; i < 355; i++) {
        mobiles.addItem( mobiles.newEID(), i, 1,
            (world.spawn_X - 256 + i ) << 5,
            (world.spawn_Y ) << 5,
            (world.spawn_Z - 2) << 5, 0x2B);
    }
        
}

//Give some items to player
void genInventory( mc__::Player& player)
{
    uint8_t slot;
    for (slot = SLOT_EQ_MAX; slot < player_inv_slots; slot++)
    {
        //Give a variety of item stacks
        player.setSlotItem(slot, (uint16_t)slot, 64, 0);
    }

}

int main(int argc, char** argv)
{
    uint32_t max_frames=0;
    bool run_limit=false;
  
    //Command line option
    switch (argc) {
        case 2:
            run_limit = true;
            max_frames = (uint32_t)(atoi( argv[1]));
            break;
        default:
            break;
    }
    
    //Default player name
    string player_name("axus");

    //Game world
    World world;

    //Game events
    Events events;

    //Generate a test world
    cout << "Generating test world..." << endl;
    genWorld(world);

    //Track entities with Mobiles object
    Mobiles mobiles(world);

    //Create some test items
    addItems(mobiles, world);
    
    //Choose entity ID for player
    uint32_t playerEID = mobiles.newEID();
    
    //Add a player at the spawn point, and make a reference to it
    Player& player = *(mobiles.addPlayer(playerEID, player_name,
        world.spawn_X << 5, world.spawn_Y << 5, world.spawn_Z << 5,
        -64/*yaw*/, 0/*pitch*/));

    //Give the player some items
    genInventory(player);

    //Create user interface to world
    cout << "Creating user interface..." << endl;
    mc__::UserInterface ui("libmc--c example",
        world, mobiles, player, events);
        
    //Run until user exits
    ui.setFramerateLimit(100);
    while (ui.run()) {
        //sf::Sleep(0.010);   //100FPS - draw time
    }
    
    //Send disconnect message
    cout << "Exiting..." << endl;
    
    //Sleep 5 seconds if run to max frames
    if (run_limit) {
        sf::sleep( sf::seconds(5));
    }
    
    //Exit with no error code
    return 0;
}
