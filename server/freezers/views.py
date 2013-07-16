from freezers.models import Freezer
from freezers.serializers import FreezerSerializer
from freezers.permissions import IsOwnerOrReadOnly
from django.http import Http404
from rest_framework.views import APIView
from rest_framework.response import Response
from rest_framework import status, generics, permissions, authentication
from django.contrib.auth.models import User
from csvwrite import write_update

class FreezerList(APIView):
    '''
    List all freezers, or create a new freezer.
    '''
    #permission_classes = (permissions.IsAuthenticated,)
    def get(self, request, format=None):
        freezers = Freezer.objects.all()
        serializer = FreezerSerializer(freezers, many=True)
        return Response(serializer.data)

    def post(self, request, format=None):
        serializer = FreezerSerializer(data=request.DATA)
        if serializer.is_valid():
            serializer.save()
            return Response(serializer.data, status=status.HTTP_201_CREATED)
        return Response(serializer.data, status=status.HTTP_400_BAD_REQUEST)

class FreezerDetail(APIView):
    '''
    Retrieve, update, or delete a freezer instance.
    '''
    #permission_classes = (permissions.IsAuthenticated,)

    def get_object(self, pk):
        try:
            return Freezer.objects.get(pk=pk)
        except Freezer.DoesNotExist:
            raise Http404

    def get(self, request, pk, format=None):
        freezer = self.get_object(pk)
        serializer = FreezerSerializer(freezer)
        return Response(serializer.data)

    def put(self, request, pk, format=None):
        freezer = self.get_object(pk)
        serializer = FreezerSerializer(freezer, data=request.DATA)
        if serializer.is_valid():
            serializer.save()
            write_update(freezer) 
            return Response(serializer.data)
        return Response(serializer.errors, status=status.HTTP_400_BAD_REQUEST)

    def delete(self, request, pk, format=None):
        freezer = self.get_object(pk)
        freezer.delete()
        return Response(status=status.HTTP_204_NO_CONTENT)
