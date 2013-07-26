from specifications.models import Specifications
from specifications.serializers import SpecificationsSerializer
from django.http import Http404
from rest_framework.views import APIView
from rest_framework.response import Response
from rest_framework import status, generics, permissions, authentication
from django.contrib.auth.models import User

class SpecificationsDetail(APIView):
    '''
    Operational specifications for the Raspberry Pi units.
    '''
    def get_specifications(self):
        try:
            return Specifications.objects.get(pk=1)
        except Specifications.DoesNotExist:
            return Specifications(read_frequency = 100000.0)

    def get(self, request, format=None):
        specifications = self.get_specifications()
        serializer = SpecificationsSerializer(specifications)
        return Response(serializer.data)

    def post(self, request, format=None):
        specifications = self.get_specifications()
        serializer = SpecificationsSerializer(specifications, data=request.DATA)
        if serializer.is_valid():
            serializer.save()
            return Response(serializer.data, status=status.HTTP_201_CREATED)
        return Response(serializer.data, status=status.HTTP_400_BAD_REQUEST)
