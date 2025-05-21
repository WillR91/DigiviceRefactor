#include "entities/DigimonDefinition.h"
#include "utils/AnimationUtils.h"
#include <SDL_log.h>

namespace Digimon {

// Static method to convert legacy enum to new definition format
DigimonDefinition DigimonDefinition::fromLegacyEnum(DigimonType type) {
    DigimonDefinition def;
    
    // Set the legacy enum value for backward compatibility
    def.legacyEnum = type;
    
    // Set up default values based on enum type
    switch (type) {        
        case DIGI_AGUMON:
            def.id = "agumon";
            def.displayName = "Agumon";
            def.digimonClass = DigimonClass::PlayerRookie;
            def.stats = {150, 15, 10, 8};
            def.spriteBaseId = "agumon"; // Updated to use new sprite ID
            def.description = "A Reptile Digimon with an appearance resembling a small dinosaur.";
            def.animations = {"Idle", "Walk", "Run", "Attack"};
            break;
              
        case DIGI_GABUMON:
            def.id = "gabumon";
            def.displayName = "Gabumon";
            def.digimonClass = DigimonClass::PlayerRookie;
            def.stats = {130, 13, 12, 7};
            def.spriteBaseId = "gabumon";
            def.description = "A Reptile Digimon with a fur pelt.";
            def.animations = {"Idle", "Walk", "Run", "Attack"};
            break;
              
        case DIGI_BIYOMON:
            def.id = "biyomon";
            def.displayName = "Biyomon";
            def.digimonClass = DigimonClass::PlayerRookie;
            def.stats = {120, 12, 8, 12};
            def.spriteBaseId = "biyomon";
            def.description = "A Bird Digimon with pink feathers.";
            def.animations = {"Idle", "Walk", "Run", "Attack"};
            break;
              
        case DIGI_GATOMON:
            def.id = "gatomon";
            def.displayName = "Gatomon";
            def.digimonClass = DigimonClass::PlayerRookie;
            def.stats = {125, 14, 9, 13};
            def.spriteBaseId = "gatomon";
            def.description = "A feline Digimon with Holy Ring on its tail.";
            def.animations = {"Idle", "Walk", "Run", "Attack"};
            break;
              
        case DIGI_GOMAMON:
            def.id = "gomamon";
            def.displayName = "Gomamon";
            def.digimonClass = DigimonClass::PlayerRookie;
            def.stats = {140, 11, 11, 9};
            def.spriteBaseId = "gomamon";
            def.description = "A Sea Animal Digimon with purple markings.";
            def.animations = {"Idle", "Walk", "Run", "Attack"};
            break;
              
        case DIGI_PALMON:
            def.id = "palmon";
            def.displayName = "Palmon";
            def.digimonClass = DigimonClass::PlayerRookie;
            def.stats = {135, 12, 10, 7};
            def.spriteBaseId = "palmon";
            def.description = "A Plant Digimon with a flower on its head.";
            def.animations = {"Idle", "Walk", "Run", "Attack"};
            break;
              
        case DIGI_TENTOMON:
            def.id = "tentomon";
            def.displayName = "Tentomon";
            def.digimonClass = DigimonClass::PlayerRookie;
            def.stats = {130, 13, 11, 8};
            def.spriteBaseId = "tentomon";
            def.description = "An Insectoid Digimon with a hard shell.";
            def.animations = {"Idle", "Walk", "Run", "Attack"};
            break;
              
        case DIGI_PATAMON:
            def.id = "patamon";
            def.displayName = "Patamon";
            def.digimonClass = DigimonClass::PlayerRookie;
            def.stats = {120, 12, 9, 11};
            def.spriteBaseId = "patamon";
            def.description = "A Mammal Digimon with bat-like wings.";
            def.animations = {"Idle", "Walk", "Run", "Attack"};
            break;
              
        case DIGI_KUWAGAMON:
            def.id = "kuwagamon";
            def.displayName = "Kuwagamon";
            def.digimonClass = DigimonClass::StandardEnemy;
            def.stats = {180, 18, 12, 9};
            def.spriteBaseId = "kuwagamon";
            def.description = "An Insectoid Digimon with large pincers and red shell.";
            def.animations = {"Idle", "Attack"};
            break;
        
        // ======== ENEMY DIGIMON DEFINITIONS - PART 1 ========
        case DIGI_ANDROMON:
            def.id = "andromon";
            def.displayName = "Andromon";
            def.digimonClass = DigimonClass::StandardEnemy;
            def.stats = {220, 20, 18, 12};
            def.spriteBaseId = "andromon";
            def.description = "A Cyborg Digimon with enhanced mechanical parts.";
            def.animations = {"Idle", "Attack"};
            break;
        
        case DIGI_APOCALYMON:
            def.id = "apocalymon";
            def.displayName = "Apocalymon";
            def.digimonClass = DigimonClass::Boss;
            def.stats = {300, 30, 25, 15};
            def.spriteBaseId = "apocalymon";
            def.description = "A monstrous Digimon born from the data of defeated and deleted Digimon.";
            def.animations = {"Idle", "Attack"};
            break;
            
        case DIGI_BAKEMON:
            def.id = "bakemon";
            def.displayName = "Bakemon";
            def.digimonClass = DigimonClass::StandardEnemy;
            def.stats = {150, 15, 10, 14};
            def.spriteBaseId = "bakemon";
            def.description = "A Ghost Digimon that loves to frighten others.";
            def.animations = {"Idle", "Attack"};
            break;
            
        case DIGI_BLOSSOMON:
            def.id = "blossomon";
            def.displayName = "Blossomon";
            def.digimonClass = DigimonClass::StandardEnemy;
            def.stats = {190, 19, 15, 8};
            def.spriteBaseId = "blossomon";
            def.description = "A Plant Digimon with dangerous ivy tendrils.";
            def.animations = {"Idle", "Attack"};
            break;
            
        case DIGI_CENTAURMON:
            def.id = "centaurmon";
            def.displayName = "Centaurmon";
            def.digimonClass = DigimonClass::StandardEnemy;
            def.stats = {200, 18, 16, 15};
            def.spriteBaseId = "centaurmon";
            def.description = "A powerful Beast Man Digimon with the upper body of a human and lower body of a horse.";
            def.animations = {"Idle", "Attack"};
            break;
            
        case DIGI_CHERRYMON:
            def.id = "cherrymon";
            def.displayName = "Cherrymon";
            def.digimonClass = DigimonClass::StandardEnemy;
            def.stats = {210, 16, 20, 5};
            def.spriteBaseId = "cherrymon";
            def.description = "An ancient Plant Digimon that resembles a large cherry tree.";
            def.animations = {"Idle", "Attack"};
            break;
            
        case DIGI_COCKATRIMON:
            def.id = "cockatrimon";
            def.displayName = "Cockatrimon";
            def.digimonClass = DigimonClass::StandardEnemy;
            def.stats = {180, 17, 14, 13};
            def.spriteBaseId = "cockatrimon";
            def.description = "A Bird Digimon that can petrify enemies with its gaze.";
            def.animations = {"Idle", "Attack"};
            break;
            
        case DIGI_COLOSSEUMGREYMON:
            def.id = "colosseumgreymon";
            def.displayName = "Colosseum Greymon";
            def.digimonClass = DigimonClass::Boss;
            def.stats = {250, 22, 18, 12};
            def.spriteBaseId = "colosseumgreymon";
            def.description = "A gladiator-like variant of Greymon trained for arena combat.";
            def.animations = {"Idle", "Attack"};
            break;
            
        case DIGI_DARKTYRANOMON:
            def.id = "darktyranomon";
            def.displayName = "DarkTyranomon";
            def.digimonClass = DigimonClass::StandardEnemy;
            def.stats = {220, 21, 15, 10};
            def.spriteBaseId = "darktyranomon";
            def.description = "A dark-colored, virus-type variant of Tyranomon with increased aggression.";
            def.animations = {"Idle", "Attack"};
            break;
            
        case DIGI_DEMIDEVIMON:
            def.id = "demidevimon";
            def.displayName = "DemiDevimon";
            def.digimonClass = DigimonClass::StandardEnemy;
            def.stats = {130, 12, 8, 15};
            def.spriteBaseId = "demidevimon";
            def.description = "A small but devious bat-like Digimon that serves evil masters.";
            def.animations = {"Idle", "Attack"};
            break;
            
        case DIGI_DERAMON:
            def.id = "deramon";
            def.displayName = "Deramon";
            def.digimonClass = DigimonClass::StandardEnemy;
            def.stats = {170, 15, 13, 12};
            def.spriteBaseId = "deramon";
            def.description = "A Bird Digimon with a bush growing on its back.";
            def.animations = {"Idle", "Attack"};
            break;
            
        // ======== ENEMY DIGIMON DEFINITIONS - PART 2 ========
        case DIGI_DEVIDRAMON:
            def.id = "devidramon";
            def.displayName = "Devidramon";
            def.digimonClass = DigimonClass::StandardEnemy;
            def.stats = {210, 20, 15, 14};
            def.spriteBaseId = "devidramon";
            def.description = "A dark Dragon Digimon with four red eyes and sharp claws.";
            def.animations = {"Idle", "Attack"};
            break;
            
        case DIGI_DEVIMON:
            def.id = "devimon";
            def.displayName = "Devimon";
            def.digimonClass = DigimonClass::StandardEnemy;
            def.stats = {200, 19, 14, 16};
            def.spriteBaseId = "devimon";
            def.description = "A fallen Angel Digimon with long arms and black wings.";
            def.animations = {"Idle", "Attack"};
            break;
            
        case DIGI_DEVIWOMON:
            def.id = "deviwomon";
            def.displayName = "Deviwomon";
            def.digimonClass = DigimonClass::StandardEnemy;
            def.stats = {185, 18, 13, 17};
            def.spriteBaseId = "deviwomon";
            def.description = "A female demonic Digimon with powerful dark magic abilities.";
            def.animations = {"Idle", "Attack"};
            break;
            
        case DIGI_DIABLOMON:
            def.id = "diablomon";
            def.displayName = "Diablomon";
            def.digimonClass = DigimonClass::Boss;
            def.stats = {270, 25, 20, 18};
            def.spriteBaseId = "diablomon";
            def.description = "A virus Digimon that attacks computer networks and multiplies rapidly.";
            def.animations = {"Idle", "Attack"};
            break;
            
        case DIGI_DIGITAMAMON:
            def.id = "digitamamon";
            def.displayName = "Digitamamon";
            def.digimonClass = DigimonClass::StandardEnemy;
            def.stats = {190, 17, 19, 12};
            def.spriteBaseId = "digitamamon";
            def.description = "A mysterious Digimon with a body like an egg and only legs and eyes visible.";
            def.animations = {"Idle", "Attack"};
            break;
            
        case DIGI_DIVERMON:
            def.id = "divermon";
            def.displayName = "Divermon";
            def.digimonClass = DigimonClass::StandardEnemy;
            def.stats = {185, 16, 14, 19};
            def.spriteBaseId = "divermon";
            def.description = "An aquatic Digimon expert in underwater combat with a harpoon.";
            def.animations = {"Idle", "Attack"};
            break;
            
        case DIGI_DOKUGUMON:
            def.id = "dokugumon";
            def.displayName = "Dokugumon";
            def.digimonClass = DigimonClass::StandardEnemy;
            def.stats = {180, 17, 15, 13};
            def.spriteBaseId = "dokugumon";
            def.description = "A poisonous spider Digimon that traps prey in its sticky web.";
            def.animations = {"Idle", "Attack"};
            break;
            
        case DIGI_DRIMOGEMON:
            def.id = "drimogemon";
            def.displayName = "Drimogemon";
            def.digimonClass = DigimonClass::StandardEnemy;
            def.stats = {195, 18, 17, 10};
            def.spriteBaseId = "drimogemon";
            def.description = "A Beast Digimon that excels at digging with its drill nose and claws.";
            def.animations = {"Idle", "Attack"};
            break;
            
        case DIGI_ELECMON:
            def.id = "elecmon";
            def.displayName = "Elecmon";
            def.digimonClass = DigimonClass::StandardEnemy;
            def.stats = {150, 15, 12, 17};
            def.spriteBaseId = "elecmon";
            def.description = "A Mammal Digimon that can generate electricity from its tail.";
            def.animations = {"Idle", "Attack"};
            break;
            
        case DIGI_ETEMON:
            def.id = "etemon";
            def.displayName = "Etemon";
            def.digimonClass = DigimonClass::Boss;
            def.stats = {220, 19, 16, 18};
            def.spriteBaseId = "etemon";
            def.description = "A Monkey Digimon who loves to perform and controls the Dark Network.";
            def.animations = {"Idle", "Attack"};
            break;
            
        case DIGI_EVILMON:
            def.id = "evilmon";
            def.displayName = "Evilmon";
            def.digimonClass = DigimonClass::StandardEnemy;
            def.stats = {160, 16, 12, 15};
            def.spriteBaseId = "evilmon";
            def.description = "A small demonic Digimon with a mischievous personality.";
            def.animations = {"Idle", "Attack"};
            break;
            
        case DIGI_FLORAMON:
            def.id = "floramon";
            def.displayName = "Floramon";
            def.digimonClass = DigimonClass::StandardEnemy;
            def.stats = {140, 13, 12, 14};
            def.spriteBaseId = "floramon";
            def.description = "A Plant Digimon with flower-like hands that emit a pleasant fragrance.";
            def.animations = {"Idle", "Attack"};
            break;
            
        case DIGI_FLYMON:
            def.id = "flymon";
            def.displayName = "Flymon";
            def.digimonClass = DigimonClass::StandardEnemy;
            def.stats = {160, 15, 11, 18};
            def.spriteBaseId = "flymon";
            def.description = "An Insectoid Digimon with a dangerous poisonous stinger.";
            def.animations = {"Idle", "Attack"};
            break;
            
        case DIGI_FRIGIMON:
            def.id = "frigimon";
            def.displayName = "Frigimon";
            def.digimonClass = DigimonClass::StandardEnemy;
            def.stats = {190, 16, 18, 10};
            def.spriteBaseId = "frigimon";
            def.description = "A Beast Digimon made of snow with ice punching attacks.";
            def.animations = {"Idle", "Attack"};
            break;
            
        case DIGI_GARBAGEMON:
            def.id = "garbagemon";
            def.displayName = "Garbagemon";
            def.digimonClass = DigimonClass::StandardEnemy;
            def.stats = {170, 14, 15, 10};
            def.spriteBaseId = "garbagemon";
            def.description = "A Digimon that lives in trash heaps and attacks with garbage.";
            def.animations = {"Idle", "Attack"};
            break;
            
        case DIGI_GAZIMON:
            def.id = "gazimon";
            def.displayName = "Gazimon";
            def.digimonClass = DigimonClass::StandardEnemy;
            def.stats = {145, 14, 12, 16};
            def.spriteBaseId = "gazimon";
            def.description = "A Mammal Digimon with sharp claws and a mischievous nature.";
            def.animations = {"Idle", "Attack"};
            break;
            
        case DIGI_GEKOMON:
            def.id = "gekomon";
            def.displayName = "Gekomon";
            def.digimonClass = DigimonClass::StandardEnemy;
            def.stats = {155, 14, 13, 13};
            def.spriteBaseId = "gekomon";
            def.description = "An Amphibian Digimon that attacks using sound waves from its horn.";
            def.animations = {"Idle", "Attack"};
            break;
            
        case DIGI_GESOMON:
            def.id = "gesomon";
            def.displayName = "Gesomon";
            def.digimonClass = DigimonClass::StandardEnemy;
            def.stats = {180, 17, 15, 11};
            def.spriteBaseId = "gesomon";
            def.description = "A Mollusk Digimon that resembles a squid with powerful tentacles.";
            def.animations = {"Idle", "Attack"};
            break;
            
        case DIGI_GIGADRAMON:
            def.id = "gigadramon";
            def.displayName = "Gigadramon";
            def.digimonClass = DigimonClass::StandardEnemy;
            def.stats = {230, 22, 18, 14};
            def.spriteBaseId = "gigadramon";
            def.description = "A Cyborg Dragon Digimon with mechanical parts and devastating attacks.";
            def.animations = {"Idle", "Attack"};
            break;
            
        case DIGI_GIZAMON:
            def.id = "gizamon";
            def.displayName = "Gizamon";
            def.digimonClass = DigimonClass::StandardEnemy;
            def.stats = {145, 14, 11, 15};
            def.spriteBaseId = "gizamon";
            def.description = "An Amphibian Digimon with a spiky back that can move quickly on both land and water.";
            def.animations = {"Idle", "Attack"};
            break;
            
        // ======== ENEMY DIGIMON DEFINITIONS - PART 3 ========
        case DIGI_GOTSUMON:
            def.id = "gotsumon";
            def.displayName = "Gotsumon";
            def.digimonClass = DigimonClass::StandardEnemy;
            def.stats = {160, 15, 17, 12};
            def.spriteBaseId = "gotsumon";
            def.description = "A Rock Digimon whose body is entirely made of stone.";
            def.animations = {"Idle", "Attack"};
            break;
            
        case DIGI_HAGURUMON:
            def.id = "hagurumon";
            def.displayName = "Hagurumon";
            def.digimonClass = DigimonClass::StandardEnemy;
            def.stats = {150, 14, 16, 10};
            def.spriteBaseId = "hagurumon";
            def.description = "A Machine Digimon shaped like a gear with simple mechanical capabilities.";
            def.animations = {"Idle", "Attack"};
            break;
            
        case DIGI_INFERMON:
            def.id = "infermon";
            def.displayName = "Infermon";
            def.digimonClass = DigimonClass::StandardEnemy;
            def.stats = {240, 24, 18, 22};
            def.spriteBaseId = "infermon";
            def.description = "An evolved form of Keramon that moves quickly through computer networks.";
            def.animations = {"Idle", "Attack"};
            break;
            
        case DIGI_KERAMON:
            def.id = "keramon";
            def.displayName = "Keramon";
            def.digimonClass = DigimonClass::StandardEnemy;
            def.stats = {170, 16, 13, 18};
            def.spriteBaseId = "keramon";
            def.description = "A virus-type Digimon that causes chaos in computer systems.";
            def.animations = {"Idle", "Attack"};
            break;
            
        case DIGI_KIWIMON:
            def.id = "kiwimon";
            def.displayName = "Kiwimon";
            def.digimonClass = DigimonClass::StandardEnemy;
            def.stats = {165, 15, 14, 17};
            def.spriteBaseId = "kiwimon";
            def.description = "A Bird Digimon resembling a kiwi bird that attacks with mini-birds.";
            def.animations = {"Idle", "Attack"};
            break;
            
        case DIGI_LEOMON:
            def.id = "leomon";
            def.displayName = "Leomon";
            def.digimonClass = DigimonClass::StandardEnemy;
            def.stats = {210, 21, 18, 15};
            def.spriteBaseId = "leomon";
            def.description = "A Beast Man Digimon with a strong sense of justice.";
            def.animations = {"Idle", "Attack"};
            break;
            
        case DIGI_MACHINEDRAMON:
            def.id = "machinedramon";
            def.displayName = "Machinedramon";
            def.digimonClass = DigimonClass::Boss;
            def.stats = {280, 27, 25, 12};
            def.spriteBaseId = "machinedramon";
            def.description = "A massive metallic Dragon Digimon with devastating firepower.";
            def.animations = {"Idle", "Attack"};
            break;
            
        case DIGI_MAMMOTHMON:
            def.id = "mammothmon";
            def.displayName = "Mammothmon";
            def.digimonClass = DigimonClass::StandardEnemy;
            def.stats = {230, 22, 20, 10};
            def.spriteBaseId = "mammothmon";
            def.description = "A large Ancient Animal Digimon resembling a wooly mammoth.";
            def.animations = {"Idle", "Attack"};
            break;
            
        case DIGI_MEGADRAMON:
            def.id = "megadramon";
            def.displayName = "Megadramon";
            def.digimonClass = DigimonClass::StandardEnemy;
            def.stats = {240, 23, 19, 16};
            def.spriteBaseId = "megadramon";
            def.description = "A Cyborg Dragon Digimon created as a weapon of mass destruction.";
            def.animations = {"Idle", "Attack"};
            break;
            
        case DIGI_MEGASEADRAMON:
            def.id = "megaseadramon";
            def.displayName = "MegaSeadramon";
            def.digimonClass = DigimonClass::StandardEnemy;
            def.stats = {225, 21, 18, 16};
            def.spriteBaseId = "megaseadramon";
            def.description = "A Sea Animal Digimon with a lightning-shaped horn and enhanced powers.";
            def.animations = {"Idle", "Attack"};
            break;
            
        case DIGI_MEKANORIMON:
            def.id = "mekanorimon";
            def.displayName = "Mekanorimon";
            def.digimonClass = DigimonClass::StandardEnemy;
            def.stats = {190, 18, 20, 8};
            def.spriteBaseId = "mekanorimon";
            def.description = "A robotic Digimon that moves using electromagnetic forces.";
            def.animations = {"Idle", "Attack"};
            break;
            
        case DIGI_MERAMON:
            def.id = "meramon";
            def.displayName = "Meramon";
            def.digimonClass = DigimonClass::StandardEnemy;
            def.stats = {190, 19, 14, 17};
            def.spriteBaseId = "meramon";
            def.description = "A Flame Digimon with a body made entirely of fire.";
            def.animations = {"Idle", "Attack"};
            break;
            
        // ======== ENEMY DIGIMON DEFINITIONS - PART 4 ========
        case DIGI_METALETEMON:
            def.id = "metaletemon";
            def.displayName = "MetalEtemon";
            def.digimonClass = DigimonClass::Boss;
            def.stats = {240, 22, 22, 16};
            def.spriteBaseId = "metaletemon";
            def.description = "A Digimon with a body coated in Chrome Digizoid, making him nearly invincible.";
            def.animations = {"Idle", "Attack"};
            break;
            
        case DIGI_METALSEADRAMON:
            def.id = "metalseadramon";
            def.displayName = "MetalSeadramon";
            def.digimonClass = DigimonClass::Boss;
            def.stats = {260, 24, 23, 15};
            def.spriteBaseId = "metalseadramon";
            def.description = "A Sea Animal Digimon with a body coated in Chrome Digizoid, leader of the Dark Masters.";
            def.animations = {"Idle", "Attack"};
            break;
            
        case DIGI_MOJYAMON:
            def.id = "mojyamon";
            def.displayName = "Mojyamon";
            def.digimonClass = DigimonClass::StandardEnemy;
            def.stats = {180, 16, 16, 12};
            def.spriteBaseId = "mojyamon";
            def.description = "A Beast Man Digimon that lives in snowy mountains.";
            def.animations = {"Idle", "Attack"};
            break;
            
        case DIGI_MONOCHROMON:
            def.id = "monochromon";
            def.displayName = "Monochromon";
            def.digimonClass = DigimonClass::StandardEnemy;
            def.stats = {200, 19, 19, 10};
            def.spriteBaseId = "monochromon";
            def.description = "A Dinosaur Digimon resembling a triceratops with a hard shell.";
            def.animations = {"Idle", "Attack"};
            break;
            
        case DIGI_MONZAEMON:
            def.id = "monzaemon";
            def.displayName = "Monzaemon";
            def.digimonClass = DigimonClass::StandardEnemy;
            def.stats = {210, 18, 17, 13};
            def.spriteBaseId = "monzaemon";
            def.description = "A Puppet Digimon resembling a giant teddy bear.";
            def.animations = {"Idle", "Attack"};
            break;
            
        case DIGI_MUSHMON:
            def.id = "mushmon";
            def.displayName = "Mushmon";
            def.digimonClass = DigimonClass::StandardEnemy;
            def.stats = {150, 14, 13, 13};
            def.spriteBaseId = "mushmon";
            def.description = "A Plant Digimon resembling a mushroom with spore-based attacks.";
            def.animations = {"Idle", "Attack"};
            break;
            
        case DIGI_MYOTISMON:
            def.id = "myotismon";
            def.displayName = "Myotismon";
            def.digimonClass = DigimonClass::Boss;
            def.stats = {230, 22, 19, 18};
            def.spriteBaseId = "myotismon";
            def.description = "A Vampire Digimon with powerful dark abilities.";
            def.animations = {"Idle", "Attack"};
            break;
            
        case DIGI_NANIMON:
            def.id = "nanimon";
            def.displayName = "Nanimon";
            def.digimonClass = DigimonClass::StandardEnemy;
            def.stats = {170, 16, 14, 14};
            def.spriteBaseId = "nanimon";
            def.description = "A bizarre Digimon with a round body and unpredictable behavior.";
            def.animations = {"Idle", "Attack"};
            break;
            
        case DIGI_NANOMON:
            def.id = "nanomon";
            def.displayName = "Nanomon";
            def.digimonClass = DigimonClass::StandardEnemy;
            def.stats = {180, 16, 17, 15};
            def.spriteBaseId = "nanomon";
            def.description = "A small Machine Digimon with exceptional technical knowledge.";
            def.animations = {"Idle", "Attack"};
            break;
            
        case DIGI_NUMEMON:
            def.id = "numemon";
            def.displayName = "Numemon";
            def.digimonClass = DigimonClass::StandardEnemy;
            def.stats = {140, 13, 11, 14};
            def.spriteBaseId = "numemon";
            def.description = "A slimy Mollusk Digimon that prefers dark and damp places.";
            def.animations = {"Idle", "Attack"};
            break;
            
        case DIGI_OGREMON:
            def.id = "ogremon";
            def.displayName = "Ogremon";
            def.digimonClass = DigimonClass::StandardEnemy;
            def.stats = {200, 20, 15, 15};
            def.spriteBaseId = "ogremon";
            def.description = "A virus-type ogre Digimon that carries a bone club.";
            def.animations = {"Idle", "Attack"};
            break;
            
        case DIGI_OTAMAMON:
            def.id = "otamamon";
            def.displayName = "Otamamon";
            def.digimonClass = DigimonClass::StandardEnemy;
            def.stats = {140, 13, 12, 15};
            def.spriteBaseId = "otamamon";
            def.description = "An Amphibian Digimon resembling a tadpole that lives in lakes and rivers.";
            def.animations = {"Idle", "Attack"};
            break;
            
        case DIGI_PARROTMON:
            def.id = "parrotmon";
            def.displayName = "Parrotmon";
            def.digimonClass = DigimonClass::StandardEnemy;
            def.stats = {200, 19, 15, 18};
            def.spriteBaseId = "parrotmon";
            def.description = "A giant Bird Digimon with the ability to generate electricity from its feathers.";
            def.animations = {"Idle", "Attack"};
            break;
            
        case DIGI_PHANTOMON:
            def.id = "phantomon";
            def.displayName = "Phantomon";
            def.digimonClass = DigimonClass::StandardEnemy;
            def.stats = {185, 18, 15, 17};
            def.spriteBaseId = "phantomon";
            def.description = "A Ghost Digimon resembling a reaper with a large scythe.";
            def.animations = {"Idle", "Attack"};
            break;
            
        case DIGI_PIEMON:
            def.id = "piemon";
            def.displayName = "Piemon";
            def.digimonClass = DigimonClass::Boss;
            def.stats = {270, 25, 22, 23};
            def.spriteBaseId = "piemon";
            def.description = "The leader of the Dark Masters with deadly sword attacks and reality-altering powers.";
            def.animations = {"Idle", "Attack"};
            break;
            
        // ======== ENEMY DIGIMON DEFINITIONS - PART 5 ========
        case DIGI_PIXIMON:
            def.id = "piximon";
            def.displayName = "Piximon";
            def.digimonClass = DigimonClass::StandardEnemy;
            def.stats = {170, 16, 15, 20};
            def.spriteBaseId = "piximon";
            def.description = "A tiny fairy Digimon known for training other Digimon with harsh methods.";
            def.animations = {"Idle", "Attack"};
            break;
            
        case DIGI_PUMPMON:
            def.id = "pumpmon";
            def.displayName = "Pumpmon";
            def.digimonClass = DigimonClass::StandardEnemy;
            def.stats = {175, 16, 14, 16};
            def.spriteBaseId = "pumpmon";
            def.description = "A Digimon with a pumpkin head who loves to play pranks.";
            def.animations = {"Idle", "Attack"};
            break;
            
        case DIGI_PUPPETMON:
            def.id = "puppetmon";
            def.displayName = "Puppetmon";
            def.digimonClass = DigimonClass::Boss;
            def.stats = {250, 24, 20, 21};
            def.spriteBaseId = "puppetmon";
            def.description = "A Puppet Digimon and one of the Dark Masters with a malicious personality.";
            def.animations = {"Idle", "Attack"};
            break;
            
        case DIGI_RAREMON:
            def.id = "raremon";
            def.displayName = "Raremon";
            def.digimonClass = DigimonClass::StandardEnemy;
            def.stats = {170, 17, 13, 10};
            def.spriteBaseId = "raremon";
            def.description = "A slimy, decaying Digimon that emits a foul odor and corrosive gases.";
            def.animations = {"Idle", "Attack"};
            break;
            
        case DIGI_REDVEGIMON:
            def.id = "redvegimon";
            def.displayName = "RedVegimon";
            def.digimonClass = DigimonClass::StandardEnemy;
            def.stats = {160, 16, 14, 12};
            def.spriteBaseId = "redvegimon";
            def.description = "A red variant of Vegimon with enhanced fighting capabilities.";
            def.animations = {"Idle", "Attack"};
            break;
            
        case DIGI_SABRELEOMON:
            def.id = "sabreleomon";
            def.displayName = "SabreLeomon";
            def.digimonClass = DigimonClass::StandardEnemy;
            def.stats = {230, 23, 19, 19};
            def.spriteBaseId = "sabreleomon";
            def.description = "A Beast Man Digimon with saber-like fangs and incredible speed.";
            def.animations = {"Idle", "Attack"};
            break;
            
        case DIGI_SCORPIOMON:
            def.id = "scorpiomon";
            def.displayName = "Scorpiomon";
            def.digimonClass = DigimonClass::StandardEnemy;
            def.stats = {210, 20, 19, 13};
            def.spriteBaseId = "scorpiomon";
            def.description = "An Ancient Crustacean Digimon resembling a scorpion with powerful pincers.";
            def.animations = {"Idle", "Attack"};
            break;
            
        case DIGI_SEADRAMON:
            def.id = "seadramon";
            def.displayName = "Seadramon";
            def.digimonClass = DigimonClass::StandardEnemy;
            def.stats = {180, 18, 15, 15};
            def.spriteBaseId = "seadramon";
            def.description = "A Sea Animal Digimon that resembles a large serpent and lives in deep water.";
            def.animations = {"Idle", "Attack"};
            break;
            
        case DIGI_SHELLMON:
            def.id = "shellmon";
            def.displayName = "Shellmon";
            def.digimonClass = DigimonClass::StandardEnemy;
            def.stats = {190, 17, 20, 8};
            def.spriteBaseId = "shellmon";
            def.description = "A Mollusk Digimon with a hard shell and water-based attacks.";
            def.animations = {"Idle", "Attack"};
            break;
            
        case DIGI_SHOGUNGEKOMON:
            def.id = "shogungekomon";
            def.displayName = "ShogunGekomon";
            def.digimonClass = DigimonClass::StandardEnemy;
            def.stats = {220, 19, 18, 10};
            def.spriteBaseId = "shogungekomon";
            def.description = "A large Amphibian Digimon with massive musical horns and royal status.";
            def.animations = {"Idle", "Attack"};
            break;
            
        case DIGI_SKULLMERAMON:
            def.id = "skullmeramon";
            def.displayName = "SkullMeramon";
            def.digimonClass = DigimonClass::StandardEnemy;
            def.stats = {210, 21, 17, 16};
            def.spriteBaseId = "skullmeramon";
            def.description = "A Flame Digimon with a skeletal body and blue fire burning all over.";
            def.animations = {"Idle", "Attack"};
            break;
            
        case DIGI_SNIMON:
            def.id = "snimon";
            def.displayName = "Snimon";
            def.digimonClass = DigimonClass::StandardEnemy;
            def.stats = {190, 19, 16, 17};
            def.spriteBaseId = "snimon";
            def.description = "An Insectoid Digimon with large sickle-like arms and excellent hunting skills.";
            def.animations = {"Idle", "Attack"};
            break;
              case DIGI_SUKAMON:
            def.id = "sukamon";
            def.displayName = "Sukamon";
            def.digimonClass = DigimonClass::StandardEnemy;
            def.stats = {150, 14, 12, 13};
            def.spriteBaseId = "sukamon";
            def.description = "A mutant Digimon made from waste data, resembling a yellow blob.";
            def.animations = {"Idle", "Attack"};
            break;
            
        case DIGI_TANKMON:
            def.id = "tankmon";
            def.displayName = "Tankmon";
            def.digimonClass = DigimonClass::StandardEnemy;
            def.stats = {215, 22, 20, 8};
            def.spriteBaseId = "tankmon";
            def.description = "A Machine Digimon resembling a tank with powerful cannon attacks.";
            def.animations = {"Idle", "Attack"};
            break;
            
        case DIGI_TUSKMON:
            def.id = "tuskmon";
            def.displayName = "Tuskmon";
            def.digimonClass = DigimonClass::StandardEnemy;
            def.stats = {205, 21, 15, 14};
            def.spriteBaseId = "tuskmon";
            def.description = "A Dinosaur Digimon with large tusks on its shoulders.";
            def.animations = {"Idle", "Attack"};
            break;
            
        case DIGI_TYRANOMON:
            def.id = "tyranomon";
            def.displayName = "Tyranomon";
            def.digimonClass = DigimonClass::StandardEnemy;
            def.stats = {210, 21, 17, 12};
            def.spriteBaseId = "tyranomon";
            def.description = "A Dinosaur Digimon resembling a T-Rex with fiery breath attacks.";
            def.animations = {"Idle", "Attack"};
            break;
            
        case DIGI_UNIMON:
            def.id = "unimon";
            def.displayName = "Unimon";
            def.digimonClass = DigimonClass::StandardEnemy;
            def.stats = {190, 18, 16, 18};
            def.spriteBaseId = "unimon";
            def.description = "A Mythical Animal Digimon resembling a winged unicorn with a helmet.";
            def.animations = {"Idle", "Attack"};
            break;
            
        case DIGI_VADEMON:
            def.id = "vademon";
            def.displayName = "Vademon";
            def.digimonClass = DigimonClass::StandardEnemy;
            def.stats = {175, 17, 15, 16};
            def.spriteBaseId = "vademon";
            def.description = "A bizarre alien-like Digimon with tentacles and a large brain.";
            def.animations = {"Idle", "Attack"};
            break;
            
        case DIGI_VEGIMON:
            def.id = "vegimon";
            def.displayName = "Vegimon";
            def.digimonClass = DigimonClass::StandardEnemy;
            def.stats = {155, 15, 13, 12};
            def.spriteBaseId = "vegimon";
            def.description = "A Plant Digimon with long vine-like tentacles.";
            def.animations = {"Idle", "Attack"};
            break;
            
        case DIGI_VENOMMYOTISMON:
            def.id = "venommyotismon";
            def.displayName = "VenomMyotismon";
            def.digimonClass = DigimonClass::Boss;
            def.stats = {280, 28, 23, 15};
            def.spriteBaseId = "venommyotismon";
            def.description = "A giant demonic form of Myotismon with greatly enhanced powers.";
            def.animations = {"Idle", "Attack"};
            break;
            
        case DIGI_WARUMONZAEMON:
            def.id = "warumonzaemon";
            def.displayName = "WaruMonzaemon";
            def.digimonClass = DigimonClass::StandardEnemy;
            def.stats = {200, 19, 16, 13};
            def.spriteBaseId = "warumonzaemon";
            def.description = "An evil version of Monzaemon with a stitched-up body and sharp claws.";
            def.animations = {"Idle", "Attack"};
            break;
            
        case DIGI_WHAMON:
            def.id = "whamon";
            def.displayName = "Whamon";
            def.digimonClass = DigimonClass::StandardEnemy;
            def.stats = {240, 18, 22, 10};
            def.spriteBaseId = "whamon";
            def.description = "A massive Sea Animal Digimon resembling a whale.";
            def.animations = {"Idle", "Attack"};
            break;
        // ======== END ENEMY DIGIMON DEFINITIONS - PART 5 ========
            
        default:
            SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "DigimonDefinition::fromLegacyEnum: Unknown DigimonType: %d", static_cast<int>(type));
            def.id = "unknown";
            def.displayName = "Unknown Digimon";
            def.digimonClass = DigimonClass::StandardEnemy;
            def.spriteBaseId = "unknown";
            def.description = "An unidentified Digimon.";
            break;
    }
    
    return def;
}

} // namespace Digimon
