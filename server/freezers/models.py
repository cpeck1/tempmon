from django.db import models
from pygments.lexers import get_all_lexers
from pygments.styles import get_all_styles
from pygments.lexers import get_lexer_by_name
from pygments.formatters.html import HtmlFormatter
from pygments import highlight

class Freezer(models.Model):
    temperature = models.FloatField()
    status = models.TextField()
    last_updated = models.DateTimeField(auto_now_add=True)

    class Meta:
        ordering = ('temperature', 'status',)
