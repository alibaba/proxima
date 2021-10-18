==============================
Pyproximabe API reference
==============================

Client
=======

Constructor
------------

.. list-table::
   :widths: 30 70
   :header-rows: 1

   * - Constructor
     - Description
   * - `Client() <#pyproximabe.Client>`_
     - ProximaSe client
   * - `AsyncClient() <#pyproximabe.AsyncClient>`_
     - ProximaSe asyncio based client, with the same interface as Client.


Methods
--------

.. list-table::
   :widths: 20 20 60
   :header-rows: 1

   * - Client
     - AsyncClient
     - Description
   * - `create_collection() <#pyproximabe.Client.create_collection>`__
     - `create_collection() <#pyproximabe.AsyncClient.create_collection>`__
     - Creates a collection.
   * - `describe_collection() <#pyproximabe.Client.describe_collection>`__
     - `describe_collection() <#pyproximabe.AsyncClient.describe_collection>`__
     - Gets collection information.
   * - `drop_collection() <#pyproximabe.Client.drop_collection>`__
     - `drop_collection() <#pyproximabe.AsyncClient.drop_collection>`__
     - Removes a collection.
   * - `stats_collection() <#pyproximabe.Client.stats_collection>`__
     - `stats_collection() <#pyproximabe.AsyncClient.stats_collection>`__
     - Gets collection statistics.
   * - `create_collection() <#pyproximabe.Client.create_collection>`__
     - `create_collection() <#pyproximabe.AsyncClient.create_collection>`__
     - Creates a collection.
   * - `list_collections() <#pyproximabe.Client.list_collections>`__
     - `list_collections() <#pyproximabe.AsyncClient.list_collections>`__
     - List all collections possibly filtered by repository name.
   * - `query() <#pyproximabe.Client.query>`__
     - `query() <#pyproximabe.AsyncClient.query>`__
     - Queries by vector.
   * - `get_document_by_key() <#pyproximabe.Client.get_document_by_key>`__
     - `get_document_by_key() <#pyproximabe.AsyncClient.get_document_by_key>`__
     - Gets document by key.
   * - `write() <#pyproximabe.Client.write>`__
     - `write() <#pyproximabe.AsyncClient.write>`__
     - Inserts/updates/deletes documents to a specified collection.
   * - `delete_document_by_keys() <#pyproximabe.Client.delete_document_by_keys>`__
     - `delete_document_by_keys() <#pyproximabe.AsyncClient.delete_document_by_keys>`__
     - Deletes documents from a collection(convenient interface).
   * - `close() <#pyproximabe.Client.close>`__
     - `close() <#pyproximabe.AsyncClient.close>`__
     - Close connection.




.. autoclass:: pyproximabe.Client
    :members: __init__, create_collection, describe_collection, drop_collection, stats_collection, list_collections, query, get_document_by_key, write, delete_document_by_keys, close
    :member-order: bysource
.. autoclass:: pyproximabe.AsyncClient
    :members: __init__, create_collection, describe_collection, drop_collection, stats_collection, list_collections, query, get_document_by_key, write, delete_document_by_keys, close
    :member-order: bysource

Types
=========

ProximaSeStatus
-----------------

.. autoclass:: pyproximabe.ProximaSeStatus
    :members: __init__, ok

DataType
-----------------

.. autoclass:: pyproximabe.DataType
  :show-inheritance:
  :members:
  :undoc-members:

IndexType
-----------------

.. autoclass:: pyproximabe.IndexType
  :show-inheritance:
  :members:
  :undoc-members:

IndexColumnParam
-----------------

.. autoclass:: pyproximabe.IndexColumnParam
    :members: __init__

CollectionConfig
-----------------

.. autoclass:: pyproximabe.CollectionConfig
    :members: __init__

CollectionInfo
-----------------

.. autoclass:: pyproximabe.CollectionInfo
    :members: __init__

DatabaseRepository
-------------------

.. autoclass:: pyproximabe.DatabaseRepository
    :members: __init__

WriteRequest
-----------------

.. autoclass:: pyproximabe.WriteRequest
    :members: __init__
.. autoclass:: pyproximabe::WriteRequest.RowMeta
    :members: __init__
.. autoclass:: pyproximabe::WriteRequest.OperationType
  :show-inheritance:
  :members:
  :undoc-members:
.. autoclass:: pyproximabe::WriteRequest.Row
    :members: __init__


Document
-----------------

.. autoclass:: pyproximabe.Document


QueryResponse
-----------------

.. autoclass:: pyproximabe.QueryResponse


CollectionStats
-----------------

.. autoclass:: pyproximabe.CollectionStats
.. autoclass:: pyproximabe.CollectionStats.SegmentStats


LsnContext
-----------------

.. autoclass:: pyproximabe.LsnContext
    :members: __init__
