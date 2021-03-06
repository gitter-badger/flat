#ifndef ENTITYCONTAINER_H_
#define ENTITYCONTAINER_H_

#include <SDL.h>
#include <iostream>
#include <map>
#include <vector>
#include <string>
#include <functional>

#include "MapArea.h"
#include "EntityShape.h"

namespace flat2d
{
	// Forward declarations
	class Entity;
	class GameData;
	class RenderData;
	class DeltatimeMonitor;
	class EntityProperties;

	typedef int Layer;
	typedef std::map<std::string, Entity*> ObjectList;
	typedef std::map<Layer, ObjectList> LayerMap;
	typedef std::map<MapArea, ObjectList> SpatialPartitionMap;
	typedef std::map<std::string, MapArea*> RenderAreas;

	/**
	 * Holds every Entity in the game. Entities registered into the
	 * EntityContainer will automatically have their update functions called
	 * and have collisionhandling performed on them if they are collidable.
	 * Registered Entity objects will be gracefully deleted when the
	 * EntityContainer is destroyed when the GameEngine exits. If you unregister
	 * an Entity you will have to dispose of it yourself in the game code.
	 *
	 * @author Linus Probert <linus.probert@gmail.com>
	 */
	class EntityContainer
	{
		private:
			// TODO(Linus): Maybe make this editable in the future?
			const int spatialPartitionExpansion = 0;
			unsigned int spatialPartitionDimension = 100;

			DeltatimeMonitor *dtMonitor = nullptr;

			ObjectList objects;
			ObjectList collidableObjects;
			ObjectList inputHandlers;
			LayerMap layeredObjects;
			SpatialPartitionMap spatialPartitionMap;
			ObjectList uninitiatedEntities;

			/* This is something I added when building a game where most of
			 * the game area was in the dark. Question is if there is actually any sense
			 * in keeping it around? The idea is that the 'developer' specifies render areas
			 * Entity objects outside the areas don't render. Does that make any sense keeping around?
			 *
			 * TODO: This could probably be removed. Seems stupid to have in the engine.
			 * */
			RenderAreas renderAreas;

			typedef std::function<bool (Entity*)> EntityProcessor;
			typedef std::function<void (Entity*)> EntityIter;

		private:
			EntityContainer(const EntityContainer&); // Don't implement
			void operator=(const EntityContainer&); // Don't implement

			void clearDeadObjects();
			void registerObjectToSpatialPartitions(Entity *entity);
			void addObjectToSpatialPartitionFor(Entity *entity, int x, int y);
			void clearObjectFromCurrentPartitions(Entity *entity);
			void clearObjectFromUnattachedPartitions(Entity *entity);
			EntityShape createBoundingBoxFor(const EntityProperties& props) const;
			void handlePossibleObjectMovement(Entity* entity);

			void reinitLayerMap();
			bool isUninitiated(const std::string& id) const;
			bool isInRenderArea(Entity*, const GameData*) const;

		public:
			static const int DEFAULT_LAYER = -1;

			explicit EntityContainer(DeltatimeMonitor* dtm) : dtMonitor(dtm) {
				reinitLayerMap();
			}

			~EntityContainer();

			/**
			 * Add a layer into the EntityContainer. Objects can then be registered
			 * to this layer. The order of the layers affects the render order of their
			 * objects. Making things render in front of or behind other objects.
			 * Layers are identified by integers. Adding the same integer multiple
			 * times has no effect.
			 *
			 * @param layer An integer acting as an identifier for the layer
			 */
			void addLayer(unsigned int);

			/**
			 * Get the registered layers keys.
			 * @return A vector containing the registered layers identifiers
			 */
			std::vector<int> getLayerKeys() const;

			/**
			 * Register an Entity to the EntityContainer. If you
			 * don't provide a layer the DEFAULT_LAYER will be used.
			 * The DEFAULT_LAYER is rendered before all other layers.
			 * @param object The Entity* to register
			 * @param layer An optional layer to register the Entity to
			 */
			void registerObject(Entity*, Layer = DEFAULT_LAYER);

			/**
			 * Unregister an entity from the EntityContainer
			 * @param entity the Entity* you want to remove
			 */
			void unregisterObject(Entity *entity);

			/**
			 * Empty the EntityContainer
			 */
			void unregisterAllObjects();

			/**
			 * Remove all Entity objects registered to a provided layer.
			 * @param layer the Layer you want to clear.
			 */
			void unregisterAllObjectsFor(Layer);

			/**
			 * Get the total amount of objects held by the EntityContainer
			 * @return The number of objects in the EntityContainer
			 */
			size_t getObjectCount() const;

			/**
			 * Get the amount of Entity objects registered to a certain layer
			 * @param layer the Layer you want to get count from
			 * @return The number of Entity objects registered to the provided Layer
			 */
			size_t getObjectCountFor(Layer);

			/**
			 * Get the number of Collidable Entity objects registerd to the EntityContainer
			 * @return The number of Entities that are collidable.
			 */
			size_t getCollidablesCount() const;

			/**
			 * Get the number of SpatialPartitions created
			 * @return The number of SpatialPartitions created
			 */
			size_t getSpatialPartitionCount() const;

			/**
			 * Recheck all registered Entity objects and see if any have become collidable
			 * after they were registered.
			 */
			void repopulateCollidables();

			/**
			 * Initiate recently added entities before continuing with
			 * main game loop. This is called from the engine and
			 * should be left untouched in game code
			 * @param gameData The GameData pointer
			 */
			void initiateEntities(const GameData *gameData);

			/**
			 * Check for input events and notify all registered Entity objects.
			 * This is called by the GameEngine and should probably not be used
			 * by game code.
			 */
			void handleObjects(const SDL_Event&, const GameData*);

			/**
			 * Call the move related callbacks on all Entity objects.
			 * This is called by the GameEngine and should probably not be used
			 * by game code.
			 */
			void moveObjects(const GameData*);

			/**
			 * Call the render related callbacks on all Entity objects.
			 * This is called by the GameEngine and should probably not be used
			 * by game code.
			 */
			void renderObjects(const GameData*) const;

			/**
			 * Set the dimension for spatial partitions.
			 * To optimize performance the EntityContainer organizes all
			 * collidable Entity objects into spatial partitions in the game space.
			 * This defaults to 100px and can be changed using this method.
			 * @param i The spatial partition dimension to use.
			 */
			void setSpatialPartitionDimension(unsigned int);

			/**
			 * Iterate all collidables in the engine and call the provided
			 * callback for each Entity. This is used by the CollisionDetector and
			 * should probably be avoided in game code.
			 * @param layer The layer to iterate
			 * @param func The EntityIter callback func to use
			 */
			void iterateCollidablesIn(Layer, EntityIter);

			/**
			 * Iterate all moving Entities in the engine and call the provided
			 * callback for each Entity.
			 * @param func The EntityIter callback func to use
			 */
			void iterateAllMovingObjects(EntityIter) const;

			/**
			 * Iterate all Entities that share a spatial partition with the provided
			 * Entity. Trigger callback for each occurence. This is used by the
			 * CollisionDetector and should be avoided in game code.
			 * @param source The Entity to operate on
			 * @param func The EntityIter callback func to use
			 */
			void iterateCollidablesFor(const Entity*, EntityIter);

			/**
			 * Check all collidables with the provided EntityProcessor.
			 * This will return the first occurence where the EntityProcessor returns
			 * true.
			 * @param func The EntityProcessor the processor to use
			 * @return The first Entity that the EntityProcessor validated
			 */
			Entity* checkAllCollidableObjects(EntityProcessor) const;

			/**
			 * Check all Entyties with the provided EntityProcessor.
			 * This will return the first occurence where the EntityProcessor returns
			 * true.
			 * @param func The EntityProcessor the processor to use
			 * @return The first Entity that the EntityProcessor validated
			 */
			Entity* checkAllObjects(EntityProcessor) const;

			/**
			 * Check all collidables within the same SpatialPartition as the provided Entity
			 * This will return the first occurence where the EntityProcessor returns
			 * true.
			 * @param source The Entity* to operate from
			 * @param func The EntityProcessor the processor to use
			 * @return The first Entity that the EntityProcessor validated
			 */
			Entity* checkCollidablesFor(const Entity*, EntityProcessor);

			/**
			 * Add a rendering area to the EntityContainer, only Entity objects
			 * within this area will be rendered during rendering.
			 * @param key A string to identify the area for retrieval
			 * @param area A MapArea object representing the render area
			 */
			void addRenderArea(const std::string& key, MapArea*);

			/**
			 * Retrieve a rendering area from the EntityContainer
			 * @param key The string key identifying the area
			 * @return The requested MapArea or null if none exists
			 */
			MapArea* getRenderArea(const std::string& key);

			/**
			 * Remove a rendering area from the EntityContainer
			 * @param key The key identifier for the area to remove
			 */
			void removeRenderArea(const std::string& key);
	};
} // namespace flat2d

#endif // ENTITYCONTAINER_H_
