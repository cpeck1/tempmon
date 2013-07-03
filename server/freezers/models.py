from django.db import models
from pygments.lexers import get_all_lexers
from pygments.styles import get_all_styles
from pygments.lexers import get_lexer_by_name
from pygments.formatters.html import HtmlFormatter
from pygments import highlight

LEXERS = [i for i in get_all_lexers() if i[1]]
LANGUAGE_CHOICES = sorted([(i[1][0], i[0]) for i in LEXERS])
STYLE_CHOICES = sorted((i, i) for i in get_all_styles())

class Freezer(models.Model):
    created = models.DateTimeField(auto_now_add=True)

    temperature = models.FloatField(default=0)
    last_update = models.DateTimeField(auto_now_add=True)

    owner = models.ForeignKey('auth.User', related_name='freezers')

    def save(self, *args, **kwargs):
        super(Freezer, self).save(*args, **kwargs)

    class Meta:
        ordering = ('created',)
    
