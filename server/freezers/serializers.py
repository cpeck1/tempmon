from django.forms import widgets
from django.db import models
from django.contrib.auth.models import User
from rest_framework import serializers
from freezers.models import Freezer, LANGUAGE_CHOICES, STYLE_CHOICES

class FreezerSerializer(serializers.HyperlinkedModelSerializer):
    owner = serializers.Field(source='owner.username')
    highlight = serializers.HyperlinkedIdentityField(
        view_name='freezer-highlight', format='html')

    class Meta:
        model = Freezer
        fields = ('url', 'owner', 'temperature', 'last_update')

class UserSerializer(serializers.HyperlinkedModelSerializer):
    freezers = serializers.ManyHyperlinkedRelatedField(
        view_name='freezer-detail')
    
    class Meta:
        model = User
        fields = ('url', 'username', 'freezers')
